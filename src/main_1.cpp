#include "IPC.h"
#include "Msg.h"
#include <unistd.h>

int main()
{
    IPC &data_store = IPC::getInstance();
    data_store.create();

    Pose pose_1;

    Data_element<int32_t> element_encoder_count_1 = {"ENC_1", "/path_arm_1", 124};
    Data_element<int32_t> element_encoder_count_2 = {"ENC_2", "/path_arm_1", 180};
    Data_element<int32_t> element_encoder_count_1_other = {"ENC_1", "/path_arm_2", 446};
    Data_element<int32_t> element_encoder_count_2_other = {"ENC_2", "/path_arm_2", 456};
    Data_element<Pose> my_pose = {"POSE_EE", "/path_arm_7"};

    data_store.register_data_element<int>(element_encoder_count_1);
    data_store.register_data_element<int>(element_encoder_count_2);
    data_store.register_data_element<int>(element_encoder_count_1_other);
    data_store.register_data_element<int>(element_encoder_count_2_other);
    data_store.register_data_element<Pose>(my_pose);

    // data_store.destroy();

    while (1)
    {
        element_encoder_count_1.value += 100;
        element_encoder_count_2.value += 100;
        element_encoder_count_1_other.value += 100;
        element_encoder_count_2_other.value += 100;
        my_pose.value.seq += 1;
        my_pose.value.translation.x += 0.1;
        my_pose.value.translation.y += 0.1;
        my_pose.value.translation.z += 0.1;
        my_pose.value.orientation.q0 += 0.01;
        my_pose.value.orientation.q1 += 0.01;
        my_pose.value.orientation.q2 += 0.01;
        my_pose.value.orientation.q3 += 0.01;
        my_pose.value.disp();

        std::cout << "---" << std::endl;
        data_store.set_data_element<int>(element_encoder_count_1);
        data_store.set_data_element<int>(element_encoder_count_2);
        data_store.set_data_element<int>(element_encoder_count_1_other);
        data_store.set_data_element<int>(element_encoder_count_2_other);
        data_store.set_data_element<Pose>(my_pose);

        sleep(2);
    }

    return 0;
}
