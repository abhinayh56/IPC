#include <iostream>
#include <pthread.h>
#include <unistd.h>     // for sleep()
#include <errno.h>      // for EBUSY

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* thread_func(void* arg) {
    int id = *(int*)arg;

    // Try to lock the mutex without blocking
    if (pthread_mutex_trylock(&lock) == 0) {
        std::cout << "Thread " << id << " got the lock\n";

        // Simulate work in the critical section
        sleep(2);

        std::cout << "Thread " << id << " releasing the lock\n";
        pthread_mutex_unlock(&lock);
    } else {
        std::cout << "Thread " << id << " could not get the lock. Skipping work.\n";
    }

    return nullptr;
}

int main() {
    pthread_t t1, t2;

    int id1 = 1, id2 = 2;

    pthread_create(&t1, nullptr, thread_func, &id1);
    sleep(1);  // ensure t1 gets the lock first
    pthread_create(&t2, nullptr, thread_func, &id2);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    pthread_mutex_destroy(&lock);

    return 0;
}
