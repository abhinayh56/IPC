#ifndef ITC_H
#define ITC_H

#include <iostream>
#include <stdint.h>
#include <map>
#include <cstring>
#include <pthread.h>
#include <vector>
#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#define SHM_NAME "/itc_shared_memory"
#define SHM_SIZE 4096

// POD only
template <typename T>
struct Data_element
{
    std::string key = "";
    std::string path = "/";
    T value;
    uint64_t index = 0;
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
        pthread_mutex_trylock(m_mutex);

        string path_key = data_element.path + "/" + data_element.key;
        std::cout << "---\n"
                  << path_key << "\n";

        auto it = m_data_element_map.find(path_key);

        if (it == m_data_element_map.end())
        {
            size_t alignment = alignof(T);
            uint64_t m_offset_required = (m_offset + alignment - 1) & ~(alignment - 1);
            size_t required_size = m_offset_required + sizeof(T);

            if (required_size > SHM_SIZE)
            {
                std::cerr << "ERROR: Not enough shared memory for new data element." << std::endl;
                pthread_mutex_unlock(m_mutex);
                return;
            }

            m_offset = m_offset_required;
            data_element.index = m_offset;
            m_data_element_map[path_key] = data_element.index;
            m_offset += sizeof(T);

            memcpy((uint8_t *)m_data_buffer + data_element.index, &data_element.value, sizeof(T));

            std::cout << "INFO: Data element set.         Index: " << data_element.index << ", Key: " << data_element.key << ", Path: " << data_element.path << ", Value: " << data_element.value << std::endl;
        }
        else
        {
            data_element.index = m_data_element_map[path_key];
            memcpy(&data_element.value, (uint8_t *)m_data_buffer + data_element.index, sizeof(T));
            std::cout << "INFO: Data element already set. Index: " << data_element.index << ", Key: " << data_element.key << ", Path: " << data_element.path << ", Value: " << data_element.value << std::endl;
        }

        pthread_mutex_unlock(m_mutex);
    }

    template <typename T>
    void set_data_element(const Data_element<T> &data_element)
    {
        if (pthread_mutex_trylock(m_mutex) == 0)
        {
            memcpy((uint8_t *)m_data_buffer + data_element.index, &data_element.value, sizeof(T));
            pthread_mutex_unlock(m_mutex);
            std::cout << "W: " << data_element.value << std::endl;
        }
        else
        {
            std::cerr << "WARN: Mutex busy during write. Skipped." << std::endl;
        }
    }

    template <typename T>
    void get_data(Data_element<T> &data_element)
    {
        if (pthread_mutex_trylock(m_mutex) == 0)
        {
            memcpy(&data_element.value, (uint8_t *)m_data_buffer + data_element.index, sizeof(T));
            pthread_mutex_unlock(m_mutex);
            std::cout << "R: " << data_element.value << std::endl;
        }
        else
        {
            std::cerr << "WARN: Mutex busy during read. Skipped." << std::endl;
        }
    }

private:
    ITC()
    {
        int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (fd == -1)
        {
            perror("shm_open");
            exit(1);
        }
        if (ftruncate(fd, SHM_SIZE) == -1)
        {
            perror("ftruncate");
            exit(1);
        }
        m_data_buffer = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (m_data_buffer == MAP_FAILED)
        {
            perror("mmap");
            exit(1);
        }

        m_mutex = static_cast<pthread_mutex_t *>((void *)m_data_buffer);
        m_offset = sizeof(pthread_mutex_t);

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(m_mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    ~ITC()
    {
        munmap(m_data_buffer, SHM_SIZE);
        shm_unlink(SHM_NAME);
    }

    void *m_data_buffer;
    uint64_t m_offset = 0;
    pthread_mutex_t *m_mutex;
    map<string, uint64_t> m_data_element_map;
};

#endif // ITC_H
