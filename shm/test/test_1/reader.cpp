#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include "common.h"

int main()
{
    // Open shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    void *ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    // Open semaphore
    sem_t *sem = sem_open(SEM_NAME, 0);

    // Wait for writer to send data
    sem_wait(sem);

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
