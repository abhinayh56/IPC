#include "SharedMemoryBus.h"
#include <thread>
#include <chrono>
#include "Msg.h"

int main()
{
    SharedMemoryBus<Pose> bus(false); // open existing shared memory

    while (true)
    {
        Pose pose;
        if (bus.read(pose))
        {
            std::cout << "R: ";
            pose.disp();
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
