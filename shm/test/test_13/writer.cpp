#include "SharedMemoryBus.h"
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char *argv[])
{
    unsigned long f = std::stoi(argv[1]);

    SharedMemoryBus<Pose> bus(true);

    Pose pose;

    while (true)
    {
        std::cout
            << "W: "
            << pose.seq << ", "
            << pose.time_stamp_ms << ", "
            << pose.translation.x << ", "
            << pose.translation.y << ", "
            << pose.translation.z << ", "
            << pose.orientation.q0 << ", "
            << pose.orientation.q1 << ", "
            << pose.orientation.q2 << ", "
            << pose.orientation.q3 << std::endl;
        // if (!std::getline(std::cin, line)) break;

        try
        {
            bus.write(pose);
            pose.seq += 1;
            pose.time_stamp_ms += 4;
            pose.translation.x += 0.1;
            pose.translation.y += 0.1;
            pose.translation.z += 0.1;
            pose.orientation.q0 += 0.01;
            pose.orientation.q1 += 0.01;
            pose.orientation.q2 += 0.01;
            pose.orientation.q3 += 0.01;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Publish failed: " << e.what() << std::endl;
        }
        usleep(1000000 / f);
    }

    return 0;
}
