#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define FILE_PATH "shared_memory.bin"
#define SIZE 4096

int main() {
    // 1. Open the existing file
    int fd = open(FILE_PATH, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    // 2. Map the file (Read-only matches how it was opened)
    char *map = (char *)mmap(NULL, SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        exit(1);
    }

    close(fd);

    // 3. Read data directly from memory
    printf("Reader: Read message: %s\n", map);

    // 4. Clean up mapping
    if (munmap(map, SIZE) == -1) {
        perror("Error unmapping");
    }

    return 0;
}
