#include <iostream>
#include <pthread.h>
#include <unistd.h>   // usleep
#include <errno.h>    // EBUSY
#include <sys/time.h> // gettimeofday

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Configurable parameters
constexpr int RETRY_DELAY_US = 200000;  // 200 ms between attempts
constexpr int MAX_TIMEOUT_US = 1000000; // 1 second max timeout

// Helper to get current time in microseconds
long long current_time_us()
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<long long>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

void *thread_func(void *arg)
{
    int id = *(int *)arg;
    long long start_time = current_time_us();
    bool acquired = false;

    while (true)
    {
        if (pthread_mutex_trylock(&lock) == 0)
        {
            long long end_time = current_time_us();
            std::cout << "Thread " << id << " acquired the lock after " << (end_time - start_time) << " µs\n";

            // Critical section
            sleep(2);

            std::cout << "Thread " << id << " releasing the lock\n";
            pthread_mutex_unlock(&lock);
            acquired = true;
            break;
        }
        else
        {
            long long now = current_time_us();
            if (now - start_time >= MAX_TIMEOUT_US)
            {
                std::cout << "Thread " << id << " timed out after " << (now - start_time) << " µs. Skipping work.\n";
                break;
            }
            usleep(RETRY_DELAY_US);
        }
    }

    return nullptr;
}

int main()
{
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_create(&t1, nullptr, thread_func, &id1);
    usleep(100000); // Let t1 grab the lock first
    pthread_create(&t2, nullptr, thread_func, &id2);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    pthread_mutex_destroy(&lock);

    return 0;
}
