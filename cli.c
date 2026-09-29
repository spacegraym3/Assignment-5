#include "kernel.h"
#include <string.h>

int generate_pagefault() {

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

    // TODO: call correct function based on mode

    // TODO: allocate the space needed for one image and load the image

    int kernel[3][3] = {{1,1,1},{1,1,1},{1,1,1}};

    // TODO: call apply kernel with 1/9 (as a float) as the normalization value
}