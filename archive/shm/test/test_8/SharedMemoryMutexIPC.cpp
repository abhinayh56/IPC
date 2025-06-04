#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <pthread.h>

const char* SHM_NAME = "/my_shm_mutex_example";
const size_t MESSAGE_SIZE = 1024;

struct SharedData {
    pthread_mutex_t mutex;
    char message[MESSAGE_SIZE];
};

void writer() {
    // Create shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Set size
    if (ftruncate(shm_fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Map memory
    SharedData* data = static_cast<SharedData*>(mmap(nullptr, sizeof(SharedData),
                                                     PROT_READ | PROT_WRITE, MAP_SHARED,
                                                     shm_fd, 0));
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Initialize mutex with shared attribute
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data->mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    // Write message
    pthread_mutex_lock(&data->mutex);
    const char* msg = "Hello from synchronized writer!";
    strncpy(data->message, msg, MESSAGE_SIZE);
    std::cout << "Writer wrote: " << data->message << std::endl;
    pthread_mutex_unlock(&data->mutex);

    // Cleanup
    munmap(data, sizeof(SharedData));
    close(shm_fd);
}

void reader() {
    // Open shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Map memory
    SharedData* data = static_cast<SharedData*>(mmap(nullptr, sizeof(SharedData),
                                                     PROT_READ | PROT_WRITE, MAP_SHARED,
                                                     shm_fd, 0));
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Read message
    pthread_mutex_lock(&data->mutex);
    std::cout << "Reader read: " << data->message << std::endl;
    pthread_mutex_unlock(&data->mutex);

    // Cleanup
    munmap(data, sizeof(SharedData));
    close(shm_fd);

    // Optionally destroy shared memory
    shm_unlink(SHM_NAME);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./SharedMemoryMutexIPC [writer|reader]" << std::endl;
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
