#include "SharedMemoryBus.h"
#include "Msg.h"

int main()
{
    SharedMemoryBusArray<Pose> bus(true);
    Pose pose;

    // Write to slot 0
    bus.write(0, pose);

    // Read from slot 0
    Pose out;
    if (bus.read(0, out))
    {
        out.disp();
    }

    return 0;
}
