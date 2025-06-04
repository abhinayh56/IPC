#include "SharedMemoryIPC.h"
#include <iostream>

int main() {
    SharedMemoryIPC ipc("/my_ipc", false);  // false = not creator

    while (true) {
        std::string msg = ipc.readMessage();
        std::cout << "Read: " << msg << std::endl;
    }

    return 0;
}
