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
struct alignas(64) Shared_data_element
{
    std::atomic<uint64_t> counter;
    bool registered = false;
    uint64_t index = 0;
    alignas(64) T data;
};

constexpr const char *SHM_NAME = "/shm_message_bus";
constexpr size_t MAX_INSTANCES = 8;
constexpr size_t SHM_SIZE = 10000; // padded total sizeces to store

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
        std::cout << "m_base: " << m_base << "sizeof(m_base): " << sizeof(m_base) << std::endl;
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

    template <typename T>
    void register_data(Shared_data_element<T> &shared_data)
    {
        if (shared_data.registered == false)
        {
            Shared_data_element<T> *p = reinterpret_cast<Shared_data_element<T> *>(m_base);
            new (&p->counter) std::atomic<uint64_t>(0);
            std::memset(&p->data, 0, sizeof(T));
            shared_data.index = reinterpret_cast<uint64_t>(p);
            shared_data.registered = true;
            m_base = reinterpret_cast<uint8_t *>(m_base + sizeof(Shared_data_element<T>));
            std::cout << "Reg (1): " << shared_data.index << std::endl;
        }
        else
        {
            std::cout << "Reg (0): " << shared_data.index << std::endl;
        }
    }

    template <typename T>
    void write(Shared_data_element<T> &data_in)
    {
        std::cout << "W : ";
        data_in.data.disp();
        Shared_data_element<T> *block = reinterpret_cast<Shared_data_element<T> *>(data_in.index);
        std::memcpy(&block->data, &data_in.data, sizeof(T));
        block->counter.fetch_add(1, std::memory_order_acq_rel);
    }

    template <typename T>
    bool read(Shared_data_element<T> &data_out)
    {
        std::cout << "R : ";
        data_out.data.disp();
        static thread_local uint64_t last_counter = {0};

        Shared_data_element<T> *block = reinterpret_cast<Shared_data_element<T> *>(data_out.index);
        uint64_t current = block->counter.load(std::memory_order_acquire);

        if (current == last_counter)
        {
            return false;
        }

        std::memcpy(&data_out.data, &block->data, sizeof(T));
        last_counter = current;
        return true;
    }

private:
    uint8_t *m_base = nullptr;
    int m_shm_fd = -1;
    bool m_is_owner = false;
    bool offset = 0;
};

#endif // SHARED_MEMORY_BUS_H
