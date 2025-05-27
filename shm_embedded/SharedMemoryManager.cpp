#include <stdint.h>
#include <string.h>
#include <iostream>

// Constants
constexpr size_t BUFFER_SIZE = 64;

// Singleton Shared Memory Manager
class SharedMemoryManager
{
private:
    static uint8_t buffer[BUFFER_SIZE]; // Private static buffer
    SharedMemoryManager() {}            // Private constructor

public:
    // Delete copy/move constructors and assignment operators
    SharedMemoryManager(const SharedMemoryManager &) = delete;
    SharedMemoryManager &operator=(const SharedMemoryManager &) = delete;

    // Accessor for the singleton instance
    static SharedMemoryManager &getInstance()
    {
        static SharedMemoryManager instance;
        return instance;
    }

    // Write data to shared buffer
    void write(const uint8_t *data, size_t length, size_t offset = 0)
    {
        if (offset + length <= BUFFER_SIZE)
        {
            memcpy(&buffer[offset], data, length);
        }
    }

    // Read data from shared buffer
    void read(uint8_t *outData, size_t length, size_t offset = 0)
    {
        if (offset + length <= BUFFER_SIZE)
        {
            memcpy(outData, &buffer[offset], length);
        }
    }
};

// Definition of static buffer
uint8_t SharedMemoryManager::buffer[BUFFER_SIZE] = {0};

// Example Producer
class SensorModule
{
public:
    void updateData()
    {
        uint8_t sensorData[4] = {1, 2, 3, 4};
        SharedMemoryManager::getInstance().write(sensorData, sizeof(sensorData));
        std::cout << "W: " << uint16_t(sensorData[0]) << ", " << uint16_t(sensorData[1]) << ", " << uint16_t(sensorData[2]) << ", " << uint16_t(sensorData[3]) << std::endl;
    }
};

// Example Consumer
class LoggerModule
{
public:
    void logData()
    {
        uint8_t data[4];
        SharedMemoryManager::getInstance().read(data, sizeof(data));
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