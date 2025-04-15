#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <chrono>
#include <thread>
#include "common.h"

int main()
{
    // Create shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    void *ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Create semaphore
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);

    int counter = 1;
    while (true)
    {
        std::string message = "Message #" + std::to_string(counter++);
        strcpy((char *)ptr, message.c_str());

        std::cout << "[Writer] Wrote: " << message << std::endl;

        sem_post(sem); // Signal reader

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait 1s
    }

    // (Optional cleanup if you make this a non-infinite loop)
    // munmap(ptr, SHM_SIZE);
    // close(shm_fd);
    // sem_close(sem);
    return 0;
}
