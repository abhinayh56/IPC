#ifndef IPC_ITC_H
#define IPC_ITC_H

#include <iostream>
#include <map>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#define SHM_NAME "/ipc_itc_shm"
#define SHM_SIZE 4096

using namespace std;

template <typename T>
struct Data_element
{
    std::string key = "";
    std::string path = "/";
    T value;
    uint64_t index = 0;
};

// Shared memory layout
struct SharedMemoryLayout
{
    pthread_mutex_t mutex;
    uint64_t offset;
    uint8_t buffer[SHM_SIZE - sizeof(pthread_mutex_t) - sizeof(uint64_t)];
};

class ITC
{
public:
    ITC(const ITC &) = delete;
    ITC &operator=(const ITC &) = delete;

    static ITC &getInstance()
    {
        static ITC instance;
        return instance;
    }

    template <typename T>
    void register_data_element(Data_element<T> &data_element)
    {
        pthread_mutex_lock(&shm_->mutex);

        string path_key = data_element.path + "/" + data_element.key;

        auto it = m_data_element_map.find(path_key);
        if (it == m_data_element_map.end())
        {
            size_t alignment = alignof(T);
            uint64_t aligned_offset = (shm_->offset + alignment - 1) & ~(alignment - 1);

            if (aligned_offset + sizeof(T) > sizeof(shm_->buffer))
            {
                std::cerr << "ERROR: Shared memory full!" << std::endl;
                pthread_mutex_unlock(&shm_->mutex);
                return;
            }

            data_element.index = aligned_offset;
            m_data_element_map[path_key] = data_element.index;
            memcpy(&shm_->buffer[data_element.index], &data_element.value, sizeof(T));
            shm_->offset = aligned_offset + sizeof(T);

            std::cout << "INFO: Registered [" << path_key << "] at offset " << data_element.index << " with value: " << data_element.value << std::endl;
        }
        else
        {
            data_element.index = it->second;
            memcpy(&data_element.value, &shm_->buffer[data_element.index], sizeof(T));
            std::cout << "INFO: Already registered [" << path_key << "] at offset " << data_element.index << " with value: " << data_element.value << std::endl;
        }

        pthread_mutex_unlock(&shm_->mutex);
    }

    template <typename T>
    void set_data_element(const Data_element<T> &data_element)
    {
        pthread_mutex_lock(&shm_->mutex);
        memcpy(&shm_->buffer[data_element.index], &data_element.value, sizeof(T));
        pthread_mutex_unlock(&shm_->mutex);
        std::cout << "W: " << data_element.value << std::endl;
    }

    template <typename T>
    void get_data(Data_element<T> &data_element)
    {
        pthread_mutex_lock(&shm_->mutex);
        memcpy(&data_element.value, &shm_->buffer[data_element.index], sizeof(T));
        pthread_mutex_unlock(&shm_->mutex);
        std::cout << "R: " << data_element.value << std::endl;
    }

private:
    ITC()
    {
        int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (fd < 0)
        {
            perror("shm_open");
            exit(1);
        }

        if (ftruncate(fd, SHM_SIZE) == -1)
        {
            perror("ftruncate");
            exit(1);
        }

        void *ptr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED)
        {
            perror("mmap");
            exit(1);
        }

        shm_ = static_cast<SharedMemoryLayout *>(ptr);

        // Initialize mutex only once (by first process)
        static bool initialized = false;
        if (!initialized && shm_->offset == 0)
        {
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&shm_->mutex, &attr);
            pthread_mutexattr_destroy(&attr);
            shm_->offset = 0;
            initialized = true;
        }
    }

    ~ITC()
    {
        munmap(shm_, SHM_SIZE);
        shm_unlink(SHM_NAME); // remove shared memory (you may disable this if other processes are using it)
    }

    SharedMemoryLayout *shm_;
    std::map<std::string, uint64_t> m_data_element_map;
};

#endif // IPC_ITC_H
