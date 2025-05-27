#include "Data_container.h"

class SensorModule
{
public:
    SensorModule(){}
    ~SensorModule(){}
    void updateData()
    {
        // Data_container::getInstance().write(sensorData, sizeof(sensorData));
        // std::cout << "W: " << uint16_t(sensorData[0]) << ", " << uint16_t(sensorData[1]) << ", " << uint16_t(sensorData[2]) << ", " << uint16_t(sensorData[3]) << std::endl;
    }

private:
    // uint8_t sensorData[4] = {14, 27, 150, 45};
    Data_element<uint8_t*> sensorData[4] = {14, 27, 150, 45}
};

// Example Consumer
class LoggerModule
{
public:
    void logData()
    {
        uint8_t data[4];
        Data_container::getInstance().read(data, sizeof(data));
        std::cout << "R: " << uint16_t(data[0]) << ", " << uint16_t(data[1]) << ", " << uint16_t(data[2]) << ", " << uint16_t(data[3]) << std::endl;
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
