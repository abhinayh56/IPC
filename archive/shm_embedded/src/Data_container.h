#ifndef DATA_CONTAINER_H
#define DATA_CONTAINER_H

#include <stdint.h>
#include <string.h>
#include <iostream>

template <typename T>
struct Data_element
{
    uint32_t offset = 0;
    uint32_t length = 0;
    T data;
};

constexpr size_t BUFFER_SIZE = 1000;

class Data_container
{
public:
    Data_container(const Data_container &) = delete;
    Data_container &operator=(const Data_container &) = delete;
    static Data_container &getInstance();
    // void write(const uint8_t *data, size_t length, size_t offset = 0);
    // void read(uint8_t *outData, size_t length, size_t offset = 0);

    template <typename T>
    void register_data(Data_element<T> &data_element)
    {
        if ((m_index + sizeof(Data_element<T>)) < BUFFER_SIZE)
        {
            data_element.offset = m_index;
            data_element.length = sizeof(Data_element<T>);
            m_index += data_element.length;
        }
        else
        {
            std::cout << "Buffer overflow!\n";
        }
    }

    template <typename T>
    void write(const Data_element<T> &data_element)
    {
        memcpy(&m_buffer[data_element.offset], &data_element.data, data_element.length);
    }

    template <typename T>
    void read(Data_element<T> &data_element)
    {
        memcpy(&data_element.data, &m_buffer[data_element.offset], data_element.length);
    }

private:
    static uint8_t m_buffer[BUFFER_SIZE];
    static uint32_t m_index;
    Data_container();
};

#endif // DATA_CONTAINER_H
