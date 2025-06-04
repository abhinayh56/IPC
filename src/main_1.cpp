#include "IPC.h"
#include <unistd.h>

int main()
{
    IPC &data_store = IPC::getInstance();
    data_store.create();

    Data_element<int32_t> element_encoder_count_1 = {"ENC_1", "/path_arm_1", 124};
    Data_element<int32_t> element_encoder_count_2 = {"ENC_2", "/path_arm_1", 180};
    Data_element<int32_t> element_encoder_count_1_other = {"ENC_1", "/path_arm_2", 446};
    Data_element<int32_t> element_encoder_count_2_other = {"ENC_2", "/path_arm_2", 456};

    data_store.register_data_element<int>(element_encoder_count_1);
    data_store.register_data_element<int>(element_encoder_count_2);
    data_store.register_data_element<int>(element_encoder_count_1_other);
    data_store.register_data_element<int>(element_encoder_count_2_other);

    data_store.destroy();

    while (1)
    {
        element_encoder_count_1.value += 100;
        element_encoder_count_2.value += 100;
        element_encoder_count_1_other.value += 100;
        element_encoder_count_2_other.value += 100;

        std::cout << "---" <<  std::endl;
        data_store.set_data_element<int>(element_encoder_count_1);
        data_store.set_data_element<int>(element_encoder_count_2);
        data_store.set_data_element<int>(element_encoder_count_1_other);
        data_store.set_data_element<int>(element_encoder_count_2_other);

        sleep(2);
    }

    return 0;
}
