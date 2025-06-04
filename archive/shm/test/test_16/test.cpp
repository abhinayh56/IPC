#include "SharedMemoryBus.h"
#include "Msg.h"

int main()
{
    SharedMemoryBusArray bus(true);
    Shared_data_element<Pose> pose_1;
    Shared_data_element<Pose> pose_2;
    Shared_data_element<Pose> pose_3;
    Shared_data_element<Pose> pose_4;
    Shared_data_element<Pose> pose_5;
    Shared_data_element<Translation> trans_1;
    Shared_data_element<Orientation> orient_1;

    bus.register_data<Pose>(pose_1);
    bus.register_data<Pose>(pose_2);
    bus.register_data<Pose>(pose_3);
    bus.register_data<Pose>(pose_4);
    bus.register_data<Pose>(pose_5);
    bus.register_data<Translation>(trans_1);
    bus.register_data<Orientation>(orient_1);

    trans_1.data = {55, 88, 11};
    orient_1.data = {0.5, 0.1, 0.15, 0.1278};

    pose_1.data.seq = 105;
    pose_1.data.translation = {10, 12, 14};
    pose_1.data.orientation = {0.1, 0.3, 0.5, 0.7};

    bus.write<Pose>(pose_1);
    bus.read<Pose>(pose_1);
    bus.write<Pose>(pose_2);
    bus.read<Pose>(pose_2);
    bus.read<Pose>(pose_1);
    bus.write<Translation>(trans_1);
    bus.write<Orientation>(orient_1);
    bus.read<Translation>(trans_1);
    bus.read<Orientation>(orient_1);

    std::cout << "Complete" << std::endl;

    return 0;
}
