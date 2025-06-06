#ifndef IPC_H
#define IPC_H

#include <iostream>
#include <stdint.h>
#include <map>
#include <string>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

using namespace std;

#define SHM_NAME "/ipc_shared_memory"
#define SHM_SIZE 4096

// Flat shared structure stored in memory (mutex + value)
template <typename T>
struct DataBlock
{
    pthread_mutex_t mutex;
    T value;
};

// Metadata for registration only (not shared)
template <typename T>
struct Data_element
{
    std::string key = "";
    std::string path = "/";
    T value;
    uint64_t index = 0;
};

class IPC
{
public:
    IPC(const IPC &) = delete;
    IPC &operator=(const IPC &) = delete;

    static IPC &getInstance()
    {
        static IPC instance;
        return instance;
    }

    void create()
    {
        int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (fd == -1)
        {
            perror("shm_open");
            exit(1);
        }
        {
            std::cout << "shm created\n";
        }
        if (ftruncate(fd, SHM_SIZE) == -1)
        {
            perror("ftruncate");
            exit(1);
        }
        {
            std::cout << "shm size set\n";
        }

        m_data_buffer = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (m_data_buffer == MAP_FAILED)
        {
            perror("mmap");
            exit(1);
        }
        close(fd);

        global_mutex = reinterpret_cast<pthread_mutex_t *>((uint8_t *)m_data_buffer);
        init_shared_mutex(global_mutex);
        m_offset = sizeof(pthread_mutex_t);
    }

    void open()
    {
        bool shm_open_flag = false;

        int fd = 0;

        while (shm_open_flag == false)
        {
            fd = shm_open(SHM_NAME, O_RDWR, 0666);
            if (fd == -1)
            {
                perror("shm_open");
                usleep(5000000);
                continue;
                // exit(1);
            }
            {
                std::cout << "shm created\n";
            }

            if (ftruncate(fd, SHM_SIZE) == -1)
            {
                perror("ftruncate");
                usleep(5000000);
                continue;
                // exit(1);
            }
            {
                std::cout << "shm size set\n";
            }

            shm_open_flag = true;
        }

        m_data_buffer = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (m_data_buffer == MAP_FAILED)
        {
            perror("mmap");
            exit(1);
        }
        close(fd);

        global_mutex = reinterpret_cast<pthread_mutex_t *>((uint8_t *)m_data_buffer);
        init_shared_mutex(global_mutex);
        m_offset = sizeof(pthread_mutex_t);
    }

    void init_shared_mutex(pthread_mutex_t *mutex)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    template <typename T>
    void register_data_element(Data_element<T> &data_element)
    {
        pthread_mutex_lock(global_mutex);

        string path_key = data_element.path + "/" + data_element.key;
        std::cout << "---\n"
                  << path_key << "\n";

        auto it = m_data_element_map.find(path_key);

        if (it == m_data_element_map.end())
        {
            size_t alignment = alignof(DataBlock<T>);
            uint64_t m_offset_required = (m_offset + alignment - 1) & ~(alignment - 1);
            size_t required_size = m_offset_required + sizeof(DataBlock<T>);

            if (required_size > SHM_SIZE)
            {
                std::cerr << "ERROR: Not enough shared memory for new data element." << std::endl;
                pthread_mutex_unlock(global_mutex);
                return;
            }

            m_offset = m_offset_required;
            data_element.index = m_offset;
            m_data_element_map[path_key] = data_element.index;
            m_offset += sizeof(DataBlock<T>);

            // Init mutex inside shared memory
            DataBlock<T> *ptr = reinterpret_cast<DataBlock<T> *>((uint8_t *)m_data_buffer + data_element.index);

            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&ptr->mutex, &attr);
            pthread_mutexattr_destroy(&attr);

            ptr->value = data_element.value;

            std::cout << "INFO: Data element set.         Index: " << data_element.index << ", Key: " << data_element.key << ", Path: " << data_element.path << ", Value: " /*<< data_element.value*/ << std::endl;
        }
        else
        {
            data_element.index = m_data_element_map[path_key];

            DataBlock<T> *ptr = reinterpret_cast<DataBlock<T> *>((uint8_t *)m_data_buffer + data_element.index);
            pthread_mutex_lock(&ptr->mutex);
            ptr->value = data_element.value;
            pthread_mutex_unlock(&ptr->mutex);
            std::cout << "INFO: Data element already set. Index: " << data_element.index << ", Key: " << data_element.key << ", Path: " << data_element.path << ", Value: " /*<< data_element.value*/ << std::endl;
        }

        pthread_mutex_unlock(global_mutex);
    }

    template <typename T>
    void access_data_element(Data_element<T> &data_element)
    {
        pthread_mutex_lock(global_mutex);

        string path_key = data_element.path + "/" + data_element.key;
        std::cout << "---\n"
                  << path_key << "\n";

        auto it = m_data_element_map.find(path_key);

        if (it == m_data_element_map.end())
        {
            size_t alignment = alignof(DataBlock<T>);
            uint64_t m_offset_required = (m_offset + alignment - 1) & ~(alignment - 1);
            size_t required_size = m_offset_required + sizeof(DataBlock<T>);

            if (required_size > SHM_SIZE)
            {
                std::cerr << "ERROR: Not enough shared memory for new data element." << std::endl;
                pthread_mutex_unlock(global_mutex);
                return;
            }

            m_offset = m_offset_required;
            data_element.index = m_offset;
            m_data_element_map[path_key] = data_element.index;
            m_offset += sizeof(DataBlock<T>);

            // Init mutex inside shared memory
            DataBlock<T> *ptr = reinterpret_cast<DataBlock<T> *>((uint8_t *)m_data_buffer + data_element.index);

            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&ptr->mutex, &attr);
            pthread_mutexattr_destroy(&attr);

            // ptr->value = data_element.value;

            std::cout << "INFO: Data element set.         Index: " << data_element.index << ", Key: " << data_element.key << ", Path: " << data_element.path << ", Value: " /*<< data_element.value*/ << std::endl;
        }
        else
        {
            data_element.index = m_data_element_map[path_key];

            DataBlock<T> *ptr = reinterpret_cast<DataBlock<T> *>((uint8_t *)m_data_buffer + data_element.index);
            pthread_mutex_lock(&ptr->mutex);
            ptr->value = data_element.value;
            pthread_mutex_unlock(&ptr->mutex);
            std::cout << "INFO: Data element already set. Index: " << data_element.index << ", Key: " << data_element.key << ", Path: " << data_element.path << ", Value: " /*<< data_element.value*/ << std::endl;
        }

        pthread_mutex_unlock(global_mutex);
    }

    template <typename T>
    void set_data_element(const Data_element<T> &data_element)
    {
        DataBlock<T> *ptr = reinterpret_cast<DataBlock<T> *>((uint8_t *)m_data_buffer + data_element.index);
        if (pthread_mutex_trylock(&ptr->mutex) == 0)
        {
            ptr->value = data_element.value;
            pthread_mutex_unlock(&ptr->mutex);
            // std::cout << "W: " << data_element.value << std::endl;
        }
        else
        {
            std::cerr << "WARN: Mutex busy during write. Skipped.\n";
        }
    }

    template <typename T>
    void get_data_element(Data_element<T> &data_element)
    {
        DataBlock<T> *ptr = reinterpret_cast<DataBlock<T> *>((uint8_t *)m_data_buffer + data_element.index);
        if (pthread_mutex_trylock(&ptr->mutex) == 0)
        {
            data_element.value = ptr->value;
            pthread_mutex_unlock(&ptr->mutex);
            // std::cout << "R: " << data_element.value << std::endl;
        }
        else
        {
            std::cerr << "WARN: Mutex busy during read. Skipped.\n";
        }
    }

    void destroy()
    {
        munmap(m_data_buffer, SHM_SIZE);
        shm_unlink(SHM_NAME);
    }

private:
    IPC() {}

    ~IPC() {}

    void *m_data_buffer;
    uint64_t m_offset = 0;
    pthread_mutex_t *global_mutex;
    map<string, uint64_t> m_data_element_map;
};

#endif // IPC_H
