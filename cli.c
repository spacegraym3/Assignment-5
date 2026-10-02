#include "kernel.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int generate_pagefault(void) {
    const int width = 2048;
    const int height = 2048;
    const size_t pixel_count = (size_t)width * height;
    const size_t pixel_bytes = pixel_count * sizeof(struct pixel);
    const size_t mapping_size = sizeof(struct image) + pixel_bytes;
    char filename[] = "/tmp/assignment5-pagefault-XXXXXX";
    struct image source = { .pixels = NULL, .width = width, .height = height };
    struct image mapped = { .pixels = NULL, .width = width, .height = height };
    int result = -1;
    int fd;
    int temp_fd;
    int advised;
    long page_size;
    int mapping_loaded = 0;

    page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) return -1;

    source.pixels = malloc(pixel_bytes);
    if (source.pixels == NULL) return -1;
    for (size_t i = 0; i < pixel_count; i++) {
        source.pixels[i].r = (int)(i & 255);
        source.pixels[i].g = (int)((i >> 8) & 255);
        source.pixels[i].b = (int)((i >> 16) & 255);
    }

    temp_fd = mkstemp(filename);
    if (temp_fd == -1) goto cleanup;
    if (close(temp_fd) == -1) {
        unlink(filename);
        goto cleanup;
    }
    if (saveimage_mmap(filename, &source) != 0) {
        unlink(filename);
        goto cleanup;
    }

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        unlink(filename);
        goto cleanup;
    }
    advised = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
    close(fd);
    if (advised != 0) {
        unlink(filename);
        goto cleanup;
    }

    if (loadimage_mmap(filename, &mapped) != 0) {
        unlink(filename);
        goto cleanup;
    }
    mapping_loaded = 1;

    volatile unsigned char *bytes = (volatile unsigned char *)mapped.pixels;
    volatile unsigned char checksum = 0;
    for (size_t offset = 0; offset < pixel_bytes; offset += (size_t)page_size)
        checksum ^= bytes[offset];
    (void)checksum;

    result = 0;

cleanup:
    if (mapping_loaded)
        munmap((char *)mapped.pixels - sizeof(struct image), mapping_size);
    free(source.pixels);
    return result;
}

int main(int argc, char** argv){
    // TODO: parse the arguments in argv. 
    // You can expect argv[1] to be the mode
    // You can expect argv[2] to be the filepath
    // You can expect argv[3] to be the integer width
    // You can expect argv[4] to be the integer height
    // You can expect argv[5] to be the output filepath.

    if(argc != 6) {
        printf("Incorrect number of arguments. Expected: ./build/image_calc <MODE=kernel|mmap|convert|uconvert|fault> <input_image> <width> <height> <output_image_path>\n");
        return -1;
    }

    const char *mode = argv[1];
    const char *input_filepath = argv[2];
    const int width = atoi(argv[3]);
    const int height = atoi(argv[4]); 
    const char *output_filepath = argv[5];

    // Check correct mode passed in
    if (strcmp(mode, "kernel") != 0 &&
        strcmp(mode, "mmap") != 0 &&
        strcmp(mode, "convert") != 0 &&
        strcmp(mode, "uconvert") != 0 &&
        strcmp(mode, "fault") != 0) {
        printf("Unknown mode '%s'. Expected kernel, mmap, convert, uconvert, or fault.\n", mode);
        return -1;
    }

    if (strcmp(mode, "fault") == 0) {
        return generate_pagefault() == 0 ? 0 : -1;
    }

    // Load image

    struct image *img = malloc(sizeof(struct image));
    if (img == NULL) {
        printf("Failed to allocate image\n");
        return -1;
    }
    img->width = width;
    img->height = height;


    int load_result = loadimage((char *)input_filepath, img);
    if (load_result != 0 || img->pixels == NULL) {
        printf("Failed to load image: %s\n", input_filepath);
        free(img);
        return -1;      
    }
    
    if (strcmp(mode, "kernel") == 0) {
        int kernel[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
        struct image *result = apply_kernel(img, (int *)kernel, 3, 1.0f / 9.0f);

        if (result == NULL) {
            printf("Failed to apply kernel\n");
            free(img->pixels);
            free(img);
            return -1;
        }
        int ret = saveimage((char *)output_filepath, result);
        free(img->pixels);
        free(img);
        free(result->pixels);
        free(result);
        return ret == 0 ? 0 : -1;
    }
    // ./cli convert images/4096x4096.bmp 4096 4096 images/4096x4096.bin
    if (strcmp(mode, "convert") == 0) {
        int result = saveimage_mmap((char *)output_filepath, img);
        free(img->pixels);
        free(img);
        return result == 0 ? 0 : -1;
    }

    int uses_mmap = strcmp(mode, "mmap") == 0;

/*
    int uses_mmap = strcmp(mode, "mmap") == 0 || strcmp(mode, "uconvert") == 0;
    int load_result = uses_mmap
        ? loadimage_mmap((char *)input_filepath, img)
        : loadimage((char *)input_filepath, img);
    if (load_result != 0 || img->pixels == NULL) {
        printf("Failed to load image: %s\n", input_filepath);
        free(img);
        return -1;
    }
        */



    if (strcmp(mode, "uconvert") == 0) {
        int result = saveimage((char *)output_filepath, img);
        munmap((char *)img->pixels - sizeof(struct image),
               sizeof(struct image) + (size_t)width * height * sizeof(struct pixel));
        free(img);
        return result == 0 ? 0 : -1;
    }

    int kernel[3][3] = {{1,1,1},{1,1,1},{1,1,1}};

    float normalize = 1.0f / 9.0f;
    struct image *result = apply_kernel(img, (int *)kernel, 3, normalize);
    if (uses_mmap) {
        munmap((char *)img->pixels - sizeof(struct image),
               sizeof(struct image) + (size_t)width * height * sizeof(struct pixel));
    } else {
        free(img->pixels);
    }
    free(img);

    if (result == NULL) {
        printf("Failed to apply kernel\n");
        return -1;
    }

    int save_result = saveimage((char *)output_filepath, result);
    free(result->pixels);
    free(result);
    return save_result == 0 ? 0 : -1;

}