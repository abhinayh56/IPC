#include "SharedMemoryBus.h"
#include "Msg.h"

int main()
{
    SharedMemoryBusArray<Pose> bus(true);
    Shared_data_element<Pose> pose_1;
    Shared_data_element<Pose> pose_2;
    Shared_data_element<Pose> pose_3;
    Shared_data_element<Pose> pose_4;
    Shared_data_element<Pose> pose_5;

    bus.register_data(pose_1);
    bus.register_data(pose_1);
    bus.register_data(pose_2);
    bus.register_data(pose_2);
    bus.register_data(pose_2);
    bus.register_data(pose_3);
    bus.register_data(pose_3);
    bus.register_data(pose_4);
    bus.register_data(pose_4);
    bus.register_data(pose_5);
    bus.register_data(pose_5);

    // Write to slot 0
    // bus.write(0, pose);

    // // Read from slot 0
    // Pose out;
    // if (bus.read(0, out))
    // {
    //     out.disp();
    // }

    std::cout << "Complete" << std::endl;

    return 0;
}
