#include "SharedMemoryBus.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char* argv[]) {
    SharedMemoryBus bus(true); // create shared memory

    unsigned long f = std::stoi(argv[1]);

    unsigned long line = 0;
    while (true) {
        std::cout << "Write: " << line << std::endl;
        // if (!std::getline(std::cin, line)) break;

        try {
            bus.publish(line);
            line++;
        } catch (const std::exception& e) {
            std::cerr << "Publish failed: " << e.what() << std::endl;
        }
        usleep(1000000/f);
    }

    return 0;
}
