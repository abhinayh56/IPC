#ifndef DATA_CONTAINER_H
#define DATA_CONTAINER_H

#include <stdint.h>
#include <string.h>
#include <iostream>

constexpr size_t BUFFER_SIZE = 1000;

class Data_container
{
public:
    Data_container(const Data_container &) = delete;
    Data_container &operator=(const Data_container &) = delete;
    static Data_container &getInstance();
    void write(const uint8_t *data, size_t length, size_t offset = 0);
    void read(uint8_t *outData, size_t length, size_t offset = 0);

private:
    static uint8_t buffer[BUFFER_SIZE];
    static uint32_t index_empty;
    Data_container();
};

#endif // DATA_CONTAINER_H
