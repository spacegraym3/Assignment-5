#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

// 1. Define your custom data structure
typedef struct {
    int id;
    char name[50];
    double balance;
} UserAccount;

int main() {
    const char *filepath = "shared_data.dat";
    
    // 2. Open the file for reading and writing (create it if it doesn't exist)
    int fd = open(filepath, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // 3. CRITICAL: Resize the file to match the size of your structure
    if (ftruncate(fd, sizeof(UserAccount)) == -1) {
        perror("Error stretching the file");
        close(fd);
        return EXIT_FAILURE;
    }

    // 4. Map the file into memory using MAP_SHARED
    UserAccount *shared_struct = (UserAccount *)mmap(
        NULL,                   // Kernel chooses the address
        sizeof(UserAccount),    // Length of the mapping
        PROT_READ | PROT_WRITE, // Read/Write permissions
        MAP_SHARED,             // Share changes across processes/file
        fd,                     // File descriptor
        0                       // Offset
    );

    if (shared_struct == MAP_FAILED) {
        perror("Error mapping the file");
        close(fd);
        return EXIT_FAILURE;
    }

    // 5. The file descriptor can be closed safely after mmap succeeds
    close(fd);

    // 6. Write to the structure directly via the pointer
    shared_struct->id = 101;
    snprintf(shared_struct->name, sizeof(shared_struct->name), "Alice Smith");
    shared_struct->balance = 5432.10;

    printf("Data written to memory-mapped structure successfully.\n");

    // 7. Clean up by unmapping the memory
    if (munmap(shared_struct, sizeof(UserAccount)) == -1) {
        perror("Error unmapping memory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
