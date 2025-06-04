#include "SharedMemoryBus.hpp"
#include <thread>
#include <chrono>

int main() {
    SharedMemoryBus bus(false); // open existing shared memory

    while (true) {
        unsigned long msg;
        if (bus.poll(msg)) {
            std::cout << "Received: " << msg << std::endl;
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
