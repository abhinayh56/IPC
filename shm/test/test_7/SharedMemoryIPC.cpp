#include <iostream>
#include <fcntl.h>     // For O_* constants
#include <sys/mman.h>  // For shm_open, mmap
#include <unistd.h>    // For ftruncate, close
#include <cstring>     // For memcpy
#include <cstdlib>     // For exit
#include <sys/stat.h>  // For mode constants

const char* SHM_NAME = "/my_shm_example";
const size_t SHM_SIZE = 1024;

void writer() {
    // Create shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open (writer)");
        exit(1);
    }

    // Set size
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Map shared memory
    void* ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (writer)");
        exit(1);
    }

    // Write to shared memory
    const char* message = "Hello from writer!";
    memcpy(ptr, message, strlen(message) + 1);
    std::cout << "Writer wrote: " << message << std::endl;

    // Cleanup
    munmap(ptr, SHM_SIZE);
    close(shm_fd);
}

void reader() {
    // Open existing shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open (reader)");
        exit(1);
    }

    // Map shared memory
    void* ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (reader)");
        exit(1);
    }

    // Read from shared memory
    char buffer[SHM_SIZE];
    memcpy(buffer, ptr, SHM_SIZE);
    std::cout << "Reader read: " << buffer << std::endl;

    // Cleanup
    munmap(ptr, SHM_SIZE);
    close(shm_fd);

    // Optionally unlink shared memory
    shm_unlink(SHM_NAME);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./SharedMemoryIPC [writer|reader]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "writer") {
        writer();
    } else if (mode == "reader") {
        reader();
    } else {
        std::cerr << "Invalid mode. Use 'writer' or 'reader'." << std::endl;
        return 1;
    }

    return 0;
}
