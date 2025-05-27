#include "SharedMemoryIPC.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    SharedMemoryIPC ipc("/my_ipc", true);  // true = creator

    for (unsigned int i = 1; i <= 100000; ++i) {
        std::string msg = "Message " + std::to_string(i);
        ipc.writeMessage(msg);
        std::cout << "Wrote: " << msg << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}
