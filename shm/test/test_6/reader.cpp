#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <csignal>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <chrono>
#include <thread>
#include "common.h"

// Globals for cleanup
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

    // Reader unlinks shared memory and semaphores
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME_DATA);
    sem_unlink(SEM_NAME_LOCK);

    std::cout << "\n[Reader Cleanup] Resources released.\n";
}

void handle_signal(int sig)
{
    std::cout << "\n[Reader] Caught signal: " << sig << std::endl;
    exit(0); // Triggers atexit
}

int main()
{
    atexit(cleanup);
    signal(SIGINT, handle_signal);

    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        std::cerr << "Failed to open shared memory\n";
        return 1;
    }

    ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        std::cerr << "Failed to mmap shared memory\n";
        return 1;
    }

    sem_data = sem_open(SEM_NAME_DATA, 0);
    sem_lock = sem_open(SEM_NAME_LOCK, 0);
    if (sem_data == SEM_FAILED || sem_lock == SEM_FAILED)
    {
        std::cerr << "Failed to open semaphores\n";
        return 1;
    }

    while (true)
    {
        if (sem_trywait(sem_data) == 0)
        {
            sem_wait(sem_lock);
            std::cout << "[Reader] Got: " << (char *)ptr << std::endl;
            sem_post(sem_lock);
        }
        else if (errno == EAGAIN)
        {
            // std::cout << "[Reader] No data yet, retrying...\n";
        }
        else
        {
            perror("sem_trywait failed");
            break;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    return 0;
}
