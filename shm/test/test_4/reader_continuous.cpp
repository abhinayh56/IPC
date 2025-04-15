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

int main()
{
    // Open shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        std::cerr << "Failed to open shared memory\n";
        return 1;
    }

    void *ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        std::cerr << "Failed to mmap shared memory\n";
        return 1;
    }

    // Open semaphore
    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED)
    {
        std::cerr << "Failed to open semaphore\n";
        return 1;
    }

    while (true)
    {
        if (sem_trywait(sem) == 0)
        {
            std::cout << "[Reader] Got: " << (char *)ptr << std::endl;
        }
        else if (errno == EAGAIN)
        {
            // std::cout << "[Reader] No data, waiting...\n";
        }
        else
        {
            perror("sem_trywait");
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // (Optional cleanup if needed)
    munmap(ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

    return 0;
}
