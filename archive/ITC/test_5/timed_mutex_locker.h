#ifndef TIMED_MUTEX_LOCKER_H
#define TIMED_MUTEX_LOCKER_H

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

class TimedMutexLocker
{
public:
    TimedMutexLocker(pthread_mutex_t *mtx, int max_retries, int retry_delay_us, int max_timeout_us)
        : mutex(mtx),
          maxRetries(max_retries),
          retryDelayUs(retry_delay_us),
          maxTimeoutUs(max_timeout_us),
          acquired(false) {}

    bool try_lock()
    {
        int attempts = 0;
        long long start = current_time_us();

        while (attempts < maxRetries)
        {
            if (pthread_mutex_trylock(mutex) == 0)
            {
                acquired = true;
                elapsed_us = current_time_us() - start;
                return true;
            }

            long long now = current_time_us();
            if (now - start >= maxTimeoutUs)
            {
                elapsed_us = now - start;
                return false;
            }

            usleep(retryDelayUs);
            ++attempts;
        }

        elapsed_us = current_time_us() - start;
        return false;
    }

    void unlock()
    {
        if (acquired)
        {
            pthread_mutex_unlock(mutex);
            acquired = false;
        }
    }

    long long elapsed_time_us() const { return elapsed_us; }

    ~TimedMutexLocker()
    {
        unlock(); // ensure mutex is released if acquired
    }

private:
    pthread_mutex_t *mutex;
    int maxRetries;
    int retryDelayUs;
    int maxTimeoutUs;
    bool acquired;
    long long elapsed_us;

    long long current_time_us() const
    {
        timeval tv;
        gettimeofday(&tv, nullptr);
        return static_cast<long long>(tv.tv_sec) * 1000000 + tv.tv_usec;
    }
};

#endif // TIMED_MUTEX_LOCKER_H
