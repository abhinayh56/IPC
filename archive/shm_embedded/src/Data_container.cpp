#include "Data_container.h"

// uint8_t Data_container::m_buffer[BUFFER_SIZE] = {0};
// uint32_t Data_container::m_index = 0;

Data_container &Data_container::getInstance()
{
    static Data_container instance;
    return instance;
}

// void Data_container::write(const uint8_t *data, size_t length, size_t offset)
// {
//     if (offset + length <= BUFFER_SIZE)
//     {
//         memcpy(&m_buffer[offset], data, length);
//     }
// }

// void Data_container::read(uint8_t *outData, size_t length, size_t offset)
// {
//     if (offset + length <= BUFFER_SIZE)
//     {
//         memcpy(outData, &m_buffer[offset], length);
//     }
// }

Data_container::Data_container()
{
}
