#pragma once
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <string>

constexpr const char* SHM_NAME = "/shm_message_bus";
constexpr size_t MESSAGE_SIZE = 256;

struct SharedData {
    std::atomic<uint64_t> counter;           // Writer increments after writing
    unsigned long message;              // Latest message
};

class SharedMemoryBus {
private:
    SharedData* data_;
    int shm_fd_;
    bool is_owner_;

public:
    SharedMemoryBus(bool create = false) {
        if (create) {
            shm_fd_ = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
            if (shm_fd_ == -1) throw std::runtime_error("shm_open failed");

            if (ftruncate(shm_fd_, sizeof(SharedData)) == -1)
                throw std::runtime_error("ftruncate failed");

            is_owner_ = true;
        } else {
            shm_fd_ = shm_open(SHM_NAME, O_RDWR, 0666);
            if (shm_fd_ == -1) throw std::runtime_error("shm_open failed");
            is_owner_ = false;
        }

        void* ptr = mmap(nullptr, sizeof(SharedData),
                         PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
        if (ptr == MAP_FAILED) throw std::runtime_error("mmap failed");

        data_ = static_cast<SharedData*>(ptr);

        if (create) {
            new (&data_->counter) std::atomic<uint64_t>(0); // placement new
            std::memset(&data_->message, 0, MESSAGE_SIZE);
        }
    }

    ~SharedMemoryBus() {
        munmap(data_, sizeof(SharedData));
        close(shm_fd_);
        if (is_owner_) shm_unlink(SHM_NAME);
    }

    // Writer API
    void publish(unsigned long& msg) {
        // if (msg.size() >= MESSAGE_SIZE)
        //     throw std::runtime_error("Message too long");

        std::memcpy(&data_->message, &msg, 8 + 1);
        std::atomic_thread_fence(std::memory_order_release);
        data_->counter.fetch_add(1, std::memory_order_relaxed);
    }

    // Reader API
    bool poll(unsigned long& out_message) {
        static thread_local uint64_t last_counter = 0;

        uint64_t current = data_->counter.load(std::memory_order_relaxed);
        if (current == last_counter) return false; // No new message

        std::atomic_thread_fence(std::memory_order_acquire);
        out_message = data_->message;
        last_counter = current;
        return true;
    }
};
