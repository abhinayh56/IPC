#include <stdint.h>
#include <iostream>

struct Translation
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct Orientation
{
    double q0 = 1.0;
    double q1 = 0.0;
    double q2 = 0.0;
    double q3 = 0.0;
};

struct Pose
{
    uint64_t seq = 0;
    uint64_t time_stamp_ms = 0;
    Translation translation;
    Orientation orientation;

    void disp()
    {
        std::cout
            << seq << ", "
            << time_stamp_ms << ", "
            << translation.x << ", "
            << translation.y << ", "
            << translation.z << ", "
            << orientation.q0 << ", "
            << orientation.q1 << ", "
            << orientation.q2 << ", "
            << orientation.q3 << std::endl;
    }
};