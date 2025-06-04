#ifndef TIMED_MUTEX_LOCKER_RT_H
#define TIMED_MUTEX_LOCKER_RT_H

#include <pthread.h>
#include <time.h>
#include <stdint.h>

class TimedMutexLockerRT
{
public:
    using LoggerFn = void(*)(const char* msg);

    TimedMutexLockerRT(pthread_mutex_t* mtx,
                       int max_retries,
                       int retry_delay_us,
                       int max_timeout_us,
                       LoggerFn logger = nullptr)
        : mutex(mtx),
          maxRetries(max_retries),
          retryDelayUs(retry_delay_us),
          maxTimeoutUs(max_timeout_us),
          acquired(false),
          elapsed_us(0),
          log_fn(logger) {}

    bool try_lock()
    {
        int attempts = 0;
        struct timespec start_time, now;
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        while (attempts < maxRetries)
        {
            if (pthread_mutex_trylock(mutex) == 0)
            {
                clock_gettime(CLOCK_MONOTONIC, &now);
                elapsed_us = time_diff_us(start_time, now);
                acquired = true;
                return true;
            }

            clock_gettime(CLOCK_MONOTONIC, &now);
            if (time_diff_us(start_time, now) >= maxTimeoutUs)
            {
                elapsed_us = time_diff_us(start_time, now);
                log("Mutex timeout");
                return false;
            }

            if (retryDelayUs > 0)
            {
                struct timespec delay = { retryDelayUs / 1000000, (retryDelayUs % 1000000) * 1000 };
                nanosleep(&delay, nullptr);
            }

            ++attempts;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed_us = time_diff_us(start_time, now);
        log("Max retry exceeded");
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

    ~TimedMutexLockerRT()
    {
        unlock(); // Ensure release
    }

private:
    pthread_mutex_t* mutex;
    int maxRetries;
    int retryDelayUs;
    int maxTimeoutUs;
    bool acquired;
    long long elapsed_us;
    LoggerFn log_fn;

    void log(const char* msg)
    {
        if (log_fn)
            log_fn(msg);
    }

    long long time_diff_us(const struct timespec& start, const struct timespec& end) const
    {
        return static_cast<long long>(end.tv_sec - start.tv_sec) * 1000000 +
               (end.tv_nsec - start.tv_nsec) / 1000;
    }
};

#endif // TIMED_MUTEX_LOCKER_RT_H
