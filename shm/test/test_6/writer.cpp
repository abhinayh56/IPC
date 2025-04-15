#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <csignal>
#include <cstdlib>
#include "common.h"

sem_t *sem_data = nullptr;
sem_t *sem_lock = nullptr;
void *ptr = nullptr;
int shm_fd = -1;

void cleanup()
{
    if (ptr)
        munmap(ptr, SHM_SIZE);
    if (shm_fd != -1)
        close(shm_fd);
    if (sem_data)
        sem_close(sem_data);
    if (sem_lock)
        sem_close(sem_lock);
    std::cout << "\n[Writer Cleanup] Resources released.\n";
}

void handle_signal(int sig)
{
    std::cout << "\n[Writer] Caught signal: " << sig << std::endl;
    exit(0); // Triggers atexit
}

int main()
{
    atexit(cleanup);
    signal(SIGINT, handle_signal);

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_data = sem_open(SEM_NAME_DATA, O_CREAT, 0666, 0);
    sem_lock = sem_open(SEM_NAME_LOCK, O_CREAT, 0666, 1);

    int counter = 1;
    while (true)
    {
        std::string msg = "Message #" + std::to_string(counter++);

        sem_wait(sem_lock);
        strcpy((char *)ptr, msg.c_str());
        sem_post(sem_lock);

        std::cout << "[Writer] Wrote: " << msg << std::endl;
        sem_post(sem_data);

        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    return 0;
}
