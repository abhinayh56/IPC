#include "IPC.h"
#include <unistd.h>

int main()
{
    IPC &data_store = IPC::getInstance();
    data_store.create();

    Data_element<int32_t> element_encoder_count_1 = {"ENC_1", "/path_arm_1", 100};
    Data_element<int32_t> element_encoder_count_2 = {"ENC_2", "/path_arm_1", 200};
    Data_element<int32_t> element_encoder_count_1_other = {"ENC_1", "/path_arm_2", 300};
    Data_element<int32_t> element_encoder_count_2_other = {"ENC_2", "/path_arm_2", 400};

    data_store.register_data_element<int>(element_encoder_count_1);
    data_store.register_data_element<int>(element_encoder_count_2);
    data_store.register_data_element<int>(element_encoder_count_1_other);
    data_store.register_data_element<int>(element_encoder_count_2_other);

    while (1)
    {
        // std::cout << "---" <<  std::endl;
        // data_store.get_data_element<int>(element_encoder_count_1);
        // data_store.get_data_element<int>(element_encoder_count_2);
        // data_store.get_data_element<int>(element_encoder_count_1_other);
        // data_store.get_data_element<int>(element_encoder_count_2_other);

        usleep(4000);
    }

    return 0;
}
