#include <iostream>
#include <pthread.h>
#include <unistd.h>   // usleep, sleep
#include <errno.h>    // EBUSY
#include <sys/time.h> // gettimeofday

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Configurable parameters
constexpr int MAX_RETRIES = 10;         // Maximum number of attempts
constexpr int RETRY_DELAY_US = 200000;  // 200 ms delay between retries
constexpr int MAX_TIMEOUT_US = 1000000; // 1 second overall timeout

// Get current time in microseconds
long long current_time_us()
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<long long>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

void *thread_func(void *arg)
{
    int id = *(int *)arg;
    int attempts = 0;
    long long start_time = current_time_us();
    bool acquired = false;

    while (attempts < MAX_RETRIES)
    {
        if (pthread_mutex_trylock(&lock) == 0)
        {
            long long end_time = current_time_us();
            std::cout << "Thread " << id << " acquired the lock on attempt " << (attempts + 1) << " after " << (end_time - start_time) << " µs\n";

            // Critical section
            sleep(2); // Simulate processing

            std::cout << "Thread " << id << " releasing the lock\n";
            pthread_mutex_unlock(&lock);
            acquired = true;
            break;
        }
        else
        {
            long long now = current_time_us();
            if ((now - start_time) >= MAX_TIMEOUT_US)
            {
                std::cout << "Thread " << id << " timed out after " << (now - start_time) << " µs. Skipping work.\n";
                break;
            }
            std::cout << "Thread " << id << " attempt " << (attempts + 1) << ": lock busy, retrying...\n";
            usleep(RETRY_DELAY_US);
        }

        attempts++;
    }

    if (!acquired && attempts >= MAX_RETRIES)
    {
        std::cout << "Thread " << id << " reached max attempts (" << MAX_RETRIES << "). Skipping work.\n";
    }

    return nullptr;
}

int main()
{
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_create(&t1, nullptr, thread_func, &id1);
    usleep(100000); // Let thread 1 start first and acquire lock
    pthread_create(&t2, nullptr, thread_func, &id2);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    pthread_mutex_destroy(&lock);

    return 0;
}
