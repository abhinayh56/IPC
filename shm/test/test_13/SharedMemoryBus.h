#ifndef SHARED_MEMORY_BUS_H
#define SHARED_MEMORY_BUS_H

#include <iostream>
#include <cstring>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <atomic>

constexpr const char *SHM_NAME = "/shm_message_bus";
constexpr size_t SHM_SIZE = 1000;

template <typename T>
struct alignas(64) Shared_data
{
    std::atomic<uint64_t> counter;
    uint64_t offset = 0;
    alignas(64) T data;
};

template <typename T>
class SharedMemoryBus
{
public:
    SharedMemoryBus(bool create = false)
    {
        if (create)
        {
            m_shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
            if (m_shm_fd == -1)
            {
                throw std::runtime_error("shm_open failed");
            }

            if (ftruncate(m_shm_fd, SHM_SIZE) == -1)
            {
                throw std::runtime_error("ftruncate failed");
            }

            m_is_owner = true;
        }
        else
        {
            m_shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
            if (m_shm_fd == -1)
            {
                throw std::runtime_error("shm_open failed");
            }
            m_is_owner = false;
        }

        void *ptr = mmap(nullptr, sizeof(Shared_data<T>), PROT_READ | PROT_WRITE, MAP_SHARED, m_shm_fd, 0);
        if (ptr == MAP_FAILED)
        {
            throw std::runtime_error("mmap failed");
        }

        m_data = static_cast<Shared_data<T> *>(ptr);

        if (create)
        {
            new (&m_data->counter) std::atomic<uint64_t>(0); // placement new
            std::memset(&m_data->data, 0, sizeof(m_data->data));
            std::memset(&m_data->offset, 0, sizeof(m_data->offset));
        }
    }

    ~SharedMemoryBus()
    {
        munmap(m_data, sizeof(Shared_data<T>));
        close(m_shm_fd);
        if (m_is_owner)
        {
            shm_unlink(SHM_NAME);
        }
    }

    void write(const T &data_in)
    {
        std::memcpy(&m_data->data, &data_in, sizeof(m_data->data));
        m_data->counter.fetch_add(1, std::memory_order_acq_rel);
        m_data->offset = 0;
    }

    bool read(T &data_out)
    {
        static thread_local uint64_t last_counter = 0;

        uint64_t current = m_data->counter.load(std::memory_order_acquire);
        if (current == last_counter)
        {
            return false; // No new message
        }

        data_out = m_data->data;
        last_counter = current;
        return true;
    }

private:
    Shared_data<T> *m_data;
    int m_shm_fd;
    bool m_is_owner;
};

#endif // SHARED_MEMORY_BUS_H
