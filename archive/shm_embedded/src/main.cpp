#include "Data_container.h"

uint8_t Data_container::m_buffer[BUFFER_SIZE] = {0};
uint32_t Data_container::m_index = 0;

Data_element<int> sensor_data;

class SensorModule
{
public:
    SensorModule() {}
    ~SensorModule() {}
    void updateData()
    {
        Data_container::getInstance().register_data(sensor_data);
        Data_container::getInstance().write(sensor_data);
        std::cout << "W: " << sensor_data.data << ", " << sensor_data.offset << ", " << sensor_data.offset << std::endl;
    }
};

// Example Consumer
class LoggerModule
{
public:
    void logData()
    {
        Data_container::getInstance().read(sensor_data);
        std::cout << "R: " << sensor_data.data << ", " << sensor_data.offset << ", " << sensor_data.offset << std::endl;
    }
};

int main()
{
    SensorModule sensor;
    LoggerModule logger;

    sensor.updateData();
    logger.logData();

    return 0;
}
