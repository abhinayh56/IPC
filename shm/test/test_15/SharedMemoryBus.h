#ifndef SHARED_MEMORY_BUS_H
#define SHARED_MEMORY_BUS_H

#include <iostream>
#include <cstring>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <atomic>
#include <stdexcept>

template <typename T>
struct alignas(64) Shared_data
{
    std::atomic<uint64_t> counter;
    uint64_t offset = 0;
    alignas(64) T data;
};

constexpr const char *SHM_NAME = "/shm_message_bus";
constexpr size_t MAX_INSTANCES = 8;
constexpr size_t SHM_SIZE = 10000; // padded total sizeces to store

template <typename T>
class SharedMemoryBusArray
{
public:
    SharedMemoryBusArray(bool create = false)
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

        void *ptr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_shm_fd, 0);
        if (ptr == MAP_FAILED)
        {
            throw std::runtime_error("mmap failed");
        }

        if (mlock(ptr, SHM_SIZE) != 0)
        {
            perror("mlock failed");
            munmap(ptr, SHM_SIZE);
            close(m_shm_fd);
            if (m_is_owner)
            {
                shm_unlink(SHM_NAME);
            }
            throw std::runtime_error("mlock failed");
        }

        m_base = static_cast<uint8_t *>(ptr);

        if (create)
        {
            for (size_t i = 0; i < MAX_INSTANCES; ++i)
            {
                auto p = get_data_block(i);
                std::cout << "p: " << i << ": " << p <<  std::endl;
                if(i>0)
                {
                    std::cout << uint64_t(get_data_block(i)) - uint64_t(get_data_block(i-1)) << std::endl;
                    std::cout << "sizeof(Shared_data<T>): " << sizeof(Shared_data<T>) << std::endl;
                }
                new (&p->counter) std::atomic<uint64_t>(0);
                std::memset(&p->data, 0, sizeof(T));
                std::memset(&p->offset, 0, sizeof(p->offset));
            }
        }
    }

    ~SharedMemoryBusArray()
    {
        munlock(m_base, SHM_SIZE);
        munmap(m_base, SHM_SIZE);
        close(m_shm_fd);
        if (m_is_owner)
        {
            shm_unlink(SHM_NAME);
        }
    }

    void write(size_t index, const T &data_in)
    {
        if (index >= MAX_INSTANCES)
        {
            throw std::out_of_range("Index out of range");
        }

        Shared_data<T> *block = get_data_block(index);
        std::memcpy(&block->data, &data_in, sizeof(T));
        block->counter.fetch_add(1, std::memory_order_acq_rel);
        block->offset = 0;
    }

    bool read(size_t index, T &data_out)
    {
        if (index >= MAX_INSTANCES)
        {
            throw std::out_of_range("Index out of range");
        }

        static thread_local uint64_t last_counter[MAX_INSTANCES] = {0};

        Shared_data<T> *block = get_data_block(index);
        uint64_t current = block->counter.load(std::memory_order_acquire);

        if (current == last_counter[index])
        {
            return false;
        }

        data_out = block->data;
        last_counter[index] = current;
        return true;
    }

private:
    Shared_data<T> *get_data_block(size_t index)
    {
        return reinterpret_cast<Shared_data<T> *>(m_base + index * sizeof(Shared_data<T>));
    }

    uint8_t *m_base = nullptr;
    int m_shm_fd = -1;
    bool m_is_owner = false;
};

#endif // SHARED_MEMORY_BUS_H
