#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include "common.h"

// #define METHOD_1
#define METHOD_2

int main()
{
    // Open shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    void *ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    // Open semaphore
    sem_t *sem = sem_open(SEM_NAME, 0);

#ifdef METHOD_1
#define METHOD_1
    int result = sem_trywait(sem);
    if (result == -1)
    {
        perror("sem_trywait failed");
        // You can handle EAGAIN or skip reading if data isn't ready yet
    }
#endif // METHOD_1

#ifdef METHOD_2
#define METHOD_2
    while (true)
    {
        if (sem_trywait(sem) == 0)
        {
            std::cout << "Got message: " << (char *)ptr << std::endl;
            break;
        }
        else
        {
            std::cout << "No message yet, waiting..." << std::endl;
            usleep(100000); // 100ms sleep
        }
    }
#endif // METHOD_2

    // Read and print message
    std::cout << "Reader got: " << (char *)ptr << std::endl;

    // Cleanup
    munmap(ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem);

    // Optionally unlink after done
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

    return 0;
}
