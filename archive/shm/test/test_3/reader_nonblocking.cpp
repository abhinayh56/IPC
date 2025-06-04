#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <cerrno>
#include <cstring>
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

    // Try to read without blocking
    int attempts = 100; // e.g. retry for ~5 seconds max
    bool received = false;

    while (attempts-- > 0)
    {
        int x = sem_trywait(sem);
        std::cout << "x: " << x << ",     ";
        if (x == 0)
        {
            std::cout << "Reader got: " << (char *)ptr << std::endl;
            received = true;
            break;
        }
        else
        {
            if (errno == EAGAIN)
            {
                std::cout << "No data yet, retrying...\n";
                usleep(100000); // Sleep 100 ms
            }
            else
            {
                perror("sem_trywait failed");
                break;
            }
        }
    }

    if (!received)
    {
        std::cout << "No message received after waiting.\n";
    }

    // Cleanup
    munmap(ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem);

    // Optionally unlink
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);

    return 0;
}
