#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <pthread.h>

const char* SHM_NAME = "/my_shm_multirw_example";
const size_t MESSAGE_SIZE = 1024;

struct SharedData {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool data_ready;
    char message[MESSAGE_SIZE];
};

void initialize_shared_data(SharedData* data) {
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data->mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&data->cond, &cond_attr);
    pthread_condattr_destroy(&cond_attr);

    data->data_ready = false;
}

void writer(const std::string& input) {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    ftruncate(shm_fd, sizeof(SharedData));

    SharedData* data = static_cast<SharedData*>(mmap(nullptr, sizeof(SharedData),
                                                     PROT_READ | PROT_WRITE, MAP_SHARED,
                                                     shm_fd, 0));
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Only initialize once (first time)
    static bool initialized = false;
    if (!initialized) {
        initialize_shared_data(data);
        initialized = true;
    }

    pthread_mutex_lock(&data->mutex);

    // Write message
    strncpy(data->message, input.c_str(), MESSAGE_SIZE);
    data->data_ready = true;

    // Notify readers
    pthread_cond_broadcast(&data->cond);

    pthread_mutex_unlock(&data->mutex);

    std::cout << "Writer wrote: " << input << std::endl;

    munmap(data, sizeof(SharedData));
    close(shm_fd);
}

void reader() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    SharedData* data = static_cast<SharedData*>(mmap(nullptr, sizeof(SharedData),
                                                     PROT_READ | PROT_WRITE, MAP_SHARED,
                                                     shm_fd, 0));
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    pthread_mutex_lock(&data->mutex);

    // Wait until data is ready
    while (!data->data_ready) {
        pthread_cond_wait(&data->cond, &data->mutex);
    }

    // Read and reset flag
    std::cout << "Reader read: " << data->message << std::endl;
    data->data_ready = false;

    pthread_mutex_unlock(&data->mutex);

    munmap(data, sizeof(SharedData));
    close(shm_fd);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./SharedMemoryMultiRW [writer <msg>|reader]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "writer") {
        if (argc != 3) {
            std::cerr << "Usage: ./SharedMemoryMultiRW writer <message>" << std::endl;
            return 1;
        }
        writer(argv[2]);
    } else if (mode == "reader") {
        reader();
    } else {
        std::cerr << "Invalid mode. Use 'writer <msg>' or 'reader'." << std::endl;
        return 1;
    }

    return 0;
}
