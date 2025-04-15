#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <chrono>
#include <thread>
#include "common.h"

int main() {
    // Create shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    void* ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Create semaphores
    sem_t* sem_data = sem_open(SEM_NAME_DATA, O_CREAT, 0666, 0); // Signals new data
    sem_t* sem_lock = sem_open(SEM_NAME_LOCK, O_CREAT, 0666, 1); // Acts as mutex

    int counter = 1;
    while (true) {
        std::string message = "Message #" + std::to_string(counter++);

        sem_wait(sem_lock); // Lock shared memory
        strcpy((char*)ptr, message.c_str());
        sem_post(sem_lock); // Unlock

        std::cout << "[Writer] Wrote: " << message << std::endl;

        sem_post(sem_data); // Signal reader

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // (Optional cleanup code here)
    // Optional cleanup
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME_DATA);
    sem_unlink(SEM_NAME_LOCK);


    return 0;
}
