#include "timed_mutex_locker.h"
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *thread_func(void *arg)
{
    int id = *(int *)arg;

    TimedMutexLocker locker(&lock, 5, 200000, 1000000); // 5 retries, 200ms delay, 1s max timeout

    if (locker.try_lock())
    {
        std::cout << "Thread " << id << " acquired lock in " << locker.elapsed_time_us() << " µs\n";
        sleep(2); // Simulate work
        std::cout << "Thread " << id << " releasing lock\n";
    }
    else
    {
        std::cout << "Thread " << id << " failed to acquire lock after " << locker.elapsed_time_us() << " µs\n";
    }

    return nullptr;
}

int main()
{
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_create(&t1, nullptr, thread_func, &id1);
    usleep(100000); // Let t1 start first
    pthread_create(&t2, nullptr, thread_func, &id2);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    pthread_mutex_destroy(&lock);

    return 0;
}
