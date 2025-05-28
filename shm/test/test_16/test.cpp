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
    bus.register_data(pose_2);
    bus.register_data(pose_3);
    bus.register_data(pose_4);
    bus.register_data(pose_5);

    pose_1.data.seq = 105;
    pose_1.data.translation = {10, 12, 14};
    pose_1.data.orientation = {0.1, 0.3, 0.5, 0.7};

    bus.write(pose_1);
    bus.read(pose_1);
    bus.write(pose_2);
    bus.read(pose_2);
    bus.read(pose_1);

    std::cout << "Complete" << std::endl;

    return 0;
}
