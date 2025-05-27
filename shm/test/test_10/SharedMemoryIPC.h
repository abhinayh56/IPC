#ifndef SHARED_MEMORY_IPC_H
#define SHARED_MEMORY_IPC_H

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>

constexpr size_t MESSAGE_SIZE = 256;
constexpr size_t BUFFER_SIZE = 8; // Number of messages in the ring buffer

struct SharedData {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    size_t read_index;
    size_t write_index;
    size_t message_count;
    char buffer[BUFFER_SIZE][MESSAGE_SIZE];
};

class SharedMemoryIPC {
public:
    SharedMemoryIPC(const std::string& shm_name, bool create);
    ~SharedMemoryIPC();

    void writeMessage(const std::string& message);
    std::string readMessage();

private:
    std::string shm_name_;
    int shm_fd_;
    SharedData* data_;
    bool creator_;
    void initializeSharedMemory();
};

#endif // SHARED_MEMORY_IPC_H
