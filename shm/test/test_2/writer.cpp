#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <unistd.h>
#include "common.h"

int main()
{
    // Create shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    void *ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Create semaphore
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);

    // Write message
    const char *message = "Hello from writer!";
    strcpy((char *)ptr, message);
    std::cout << "Writer wrote: " << message << std::endl;

    usleep(5000000);

    // Signal reader
    sem_post(sem);

    // Cleanup
    munmap(ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem);
    return 0;
}
