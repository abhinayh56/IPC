#include "IPC.h"
#include "Msg.h"
#include <unistd.h>

int main()
{
    IPC &data_store = IPC::getInstance();
    data_store.open();

    Pose pose_1;

    Data_element<int32_t> element_encoder_count_1 = {"ENC_1", "/path_arm_1", 100};
    Data_element<int32_t> element_encoder_count_2 = {"ENC_2", "/path_arm_1", 200};
    Data_element<int32_t> element_encoder_count_1_other = {"ENC_1", "/path_arm_2", 300};
    Data_element<int32_t> element_encoder_count_2_other = {"ENC_2", "/path_arm_2", 400};
    Data_element<Pose> my_pose = {"POSE_EE", "/path_arm_7"};

    data_store.map_data_element<int>(element_encoder_count_1);
    data_store.map_data_element<int>(element_encoder_count_2);
    data_store.map_data_element<int>(element_encoder_count_1_other);
    data_store.map_data_element<int>(element_encoder_count_2_other);
    data_store.map_data_element<Pose>(my_pose);

    int last = 0;

    while (1)
    {
        data_store.get_data_element<int>(element_encoder_count_1);
        data_store.get_data_element<int>(element_encoder_count_2);
        data_store.get_data_element<int>(element_encoder_count_1_other);
        data_store.get_data_element<int>(element_encoder_count_2_other);
        data_store.get_data_element<Pose>(my_pose);

        if (my_pose.value.seq != last)
        {
            std::cout << "---" << std::endl;
            my_pose.value.disp();
            last = my_pose.value.seq;
        }

        usleep(10000);
    }

    return 0;
}
