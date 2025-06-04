#include "TimedMutexLockerRT.h"
#include <pthread.h>
#include <stdio.h>
#include <thread>

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void realtime_logger(const char* msg)
{
    // Do not use printf in hard-RT thread!
    // This is only for demonstration outside real-time thread
    fprintf(stderr, "LOG: %s\n", msg);
}

void task(int id)
{
    TimedMutexLockerRT locker(&mtx, 10, 1000, 3000, realtime_logger);
    if (locker.try_lock()) {
        // Do RT-safe work
        // (e.g., PDO access, sensor update)
    }
}

int main()
{
    std::thread t1(task, 1);
    std::thread t2(task, 2);

    t1.join();
    t2.join();
    pthread_mutex_destroy(&mtx);
    return 0;
}
