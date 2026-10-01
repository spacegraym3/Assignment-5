#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define FILE_PATH "shared_memory.bin"
#define SIZE 4096 // 4KB page size

int main() {
    // 1. Open the file for reading and writing (create if it doesn't exist)
    int fd = open(FILE_PATH, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    // 2. Ensure the file is large enough (mmap cannot expand file size dynamically)
    if (ftruncate(fd, SIZE) == -1) {
        perror("Error setting file size");
        close(fd);
        exit(1);
    }

    // 3. Map the file into memory using MAP_SHARED
    char *map = (char *)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        exit(1);
    }

    // 4. File descriptor can be closed safely after mapping
    close(fd);

    // 5. Write data directly to the memory region
    strcpy(map, "Hello from the Writer Process!");
    printf("Writer: Wrote to memory. Press Enter to exit...\n");
    getchar();

    // 6. Clean up mapping
    if (munmap(map, SIZE) == -1) {
        perror("Error unmapping");
    }

    return 0;
}
