#include <iostream>
#include <pthread.h>
#include <unistd.h>     // for usleep()
#include <errno.h>      // for EBUSY

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Configurable parameters
constexpr int MAX_RETRIES = 5000;
constexpr int RETRY_DELAY_US = 200000; // 200 ms

void* thread_func(void* arg) {
    int id = *(int*)arg;
    int attempts = 0;
    bool acquired = false;

    while (attempts < MAX_RETRIES) {
        if (pthread_mutex_trylock(&lock) == 0) {
            std::cout << "Thread " << id << " acquired the lock on attempt " << (attempts + 1) << "\n";
            // Critical section
            sleep(2);
            std::cout << "Thread " << id << " releasing the lock\n";
            pthread_mutex_unlock(&lock);
            acquired = true;
            break;
        } else {
            std::cout << "Thread " << id << " attempt " << (attempts + 1) << ": lock busy, retrying...\n";
            usleep(RETRY_DELAY_US); // sleep before retrying
        }
        attempts++;
    }

    if (!acquired) {
        std::cout << "Thread " << id << " could not acquire the lock after " << MAX_RETRIES << " attempts. Skipping.\n";
    }

    return nullptr;
}

int main() {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_create(&t1, nullptr, thread_func, &id1);
    usleep(100000);  // 100 ms offset to let t1 grab the lock first
    pthread_create(&t2, nullptr, thread_func, &id2);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    pthread_mutex_destroy(&lock);

    return 0;
}
