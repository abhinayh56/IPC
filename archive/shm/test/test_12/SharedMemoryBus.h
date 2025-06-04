#pragma once
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <string>

constexpr const char *SHM_NAME = "/shm_message_bus";
constexpr size_t MESSAGE_SIZE = 256;

// Ensure alignment to avoid false sharing and improve cache coherency
struct alignas(64) SharedData
{
    alignas(64) std::atomic<uint64_t> counter; // Writer increments after writing
    alignas(64) unsigned long message;         // Latest message
};

class SharedMemoryBus
{
private:
    SharedData *data_;
    int shm_fd_;
    bool is_owner_;

public:
    SharedMemoryBus(bool create = false);
    ~SharedMemoryBus();
    // Writer API
    void publish(const unsigned long &msg);
    // Reader API
    bool poll(unsigned long &out_message);
};
