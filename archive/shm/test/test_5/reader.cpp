#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <cerrno>
#include <cstring>
#include <chrono>
#include <thread>
#include "common.h"

int main() {
    // Open shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cerr << "Failed to open shared memory\n";
        return 1;
    }

    void* ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        std::cerr << "Failed to mmap shared memory\n";
        return 1;
    }

    // Open semaphores
    sem_t* sem_data = sem_open(SEM_NAME_DATA, 0);
    sem_t* sem_lock = sem_open(SEM_NAME_LOCK, 0);

    if (sem_data == SEM_FAILED || sem_lock == SEM_FAILED) {
        std::cerr << "Failed to open semaphores\n";
        return 1;
    }

    while (true) {
        if (sem_trywait(sem_data) == 0) {
            sem_wait(sem_lock); // Lock before read
            std::cout << "[Reader] Got: " << (char*)ptr << std::endl;
            sem_post(sem_lock); // Unlock
        } else if (errno == EAGAIN) {
            std::cout << "[Reader] No data yet, retrying...\n";
        } else {
            perror("sem_trywait failed");
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // (Optional cleanup)
    // Optional cleanup
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME_DATA);
    sem_unlink(SEM_NAME_LOCK);


    return 0;
}
