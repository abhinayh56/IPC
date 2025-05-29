#ifndef ITC_H
#define ITC_H

#include <iostream>
#include <stdint.h>
#include <map>
#include <cstring>

using namespace std;

#define ITC_BUFFER_LEN 15

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
        static ITC instance; // Initialized on first call, thread-safe in C++11+
        return instance;
    }

    template <typename T>
    void register_data_element(Data_element<T> &data_element)
    {
        string path_key = data_element.path + "/" + data_element.key;
        std::cout << "---\n"
                  << path_key << "\n";

        auto it = m_data_element_map.find(path_key);

        if (it == m_data_element_map.end())
        {
            if (m_offset + sizeof(T) > ITC_BUFFER_LEN)
            {
                std::cout << "ERROR: Failed to register. ITC buffer overflow! " << path_key << std::endl;
            }
            else
            {
                size_t alignment = alignof(T);
                m_offset = (m_offset + alignment - 1) & ~(alignment - 1); // align up

                data_element.index = m_offset;
                m_data_element_map.insert({path_key, data_element.index});
                m_offset += sizeof(T);
                memcpy(&m_data_buffer[data_element.index], &data_element.value, sizeof(T));

                std::cout << "INFO: Data element set.         Index: " << data_element.index << ", Key: " << data_element.key << ", Path: " << data_element.path << ", Value: " << data_element.value << std::endl;
            }
        }
        else
        {
            data_element.index = m_data_element_map[path_key];
            memcpy(&data_element.value, &m_data_buffer[data_element.index], sizeof(T));
            std::cout << "INFO: Data element already set. Index: " << data_element.index << ", Key: " << data_element.key << ", Path: " << data_element.path << ", Value: " << data_element.value << std::endl;
        }
    }

    template <typename T>
    void set_data_element(const Data_element<T> &data_element)
    {
        memcpy(&m_data_buffer[data_element.index], &data_element.value, sizeof(T));
        std::cout << "W: " << data_element.value << std::endl;
    }

    template <typename T>
    void get_data(Data_element<T> &data_element)
    {
        memcpy(&data_element.value, &m_data_buffer[data_element.index], sizeof(T));
        std::cout << "R: " << data_element.value << std::endl;
    }

private:
    ITC()
    {
    }

    ~ITC()
    {
    }

    uint8_t m_data_buffer[ITC_BUFFER_LEN];
    uint64_t m_offset = 0;

    map<string, uint64_t> m_data_element_map; // key (string name of data element) : value (pointer of data element)
};

#endif // ITC_H
