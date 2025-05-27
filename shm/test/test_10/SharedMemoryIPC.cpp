#include "SharedMemoryIPC.h"

SharedMemoryIPC::SharedMemoryIPC(const std::string& shm_name, bool create)
    : shm_name_(shm_name), creator_(create) {
    shm_fd_ = shm_open(shm_name_.c_str(), create ? (O_CREAT | O_RDWR) : O_RDWR, 0666);
    if (shm_fd_ == -1) throw std::runtime_error("Failed to open shared memory");

    if (create) {
        if (ftruncate(shm_fd_, sizeof(SharedData)) == -1)
            throw std::runtime_error("Failed to set shared memory size");
    }

    data_ = static_cast<SharedData*>(mmap(nullptr, sizeof(SharedData),
                                          PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0));
    if (data_ == MAP_FAILED)
        throw std::runtime_error("Failed to mmap shared memory");

    if (create) {
        initializeSharedMemory();
    }
}

void SharedMemoryIPC::initializeSharedMemory() {
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data_->mutex, &mattr);
    pthread_mutexattr_destroy(&mattr);

    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&data_->cond, &cattr);
    pthread_condattr_destroy(&cattr);

    data_->read_index = 0;
    data_->write_index = 0;
    data_->message_count = 0;
}

void SharedMemoryIPC::writeMessage(const std::string& message) {
    pthread_mutex_lock(&data_->mutex);

    while (data_->message_count == BUFFER_SIZE) {
        // Buffer is full, wait for readers
        pthread_cond_wait(&data_->cond, &data_->mutex);
    }

    strncpy(data_->buffer[data_->write_index], message.c_str(), MESSAGE_SIZE);
    data_->write_index = (data_->write_index + 1) % BUFFER_SIZE;
    ++data_->message_count;

    pthread_cond_broadcast(&data_->cond);
    pthread_mutex_unlock(&data_->mutex);
}

std::string SharedMemoryIPC::readMessage() {
    pthread_mutex_lock(&data_->mutex);

    while (data_->message_count == 0) {
        // Buffer is empty, wait for writers
        pthread_cond_wait(&data_->cond, &data_->mutex);
    }

    std::string msg(data_->buffer[data_->read_index]);
    data_->read_index = (data_->read_index + 1) % BUFFER_SIZE;
    --data_->message_count;

    pthread_cond_broadcast(&data_->cond);
    pthread_mutex_unlock(&data_->mutex);

    return msg;
}

SharedMemoryIPC::~SharedMemoryIPC() {
    munmap(data_, sizeof(SharedData));
    close(shm_fd_);

    if (creator_) {
        shm_unlink(shm_name_.c_str());
    }
}
