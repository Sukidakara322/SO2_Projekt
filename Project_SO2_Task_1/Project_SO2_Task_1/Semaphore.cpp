#include "Semaphore.h"

void initSemaphore(Semaphore& s, int initValue) {
    // How much threads can go further
    s.value = initValue;
    // Mutex to protect value
    pthread_mutex_init(&s.mutex, nullptr);
    // Cond to block/unblock threads
    pthread_cond_init(&s.cond, nullptr);
}

void waitSemaphore(Semaphore& s) {
    pthread_mutex_lock(&s.mutex);
    // If value <= 0, wait
    while (s.value <= 0) {
        pthread_cond_wait(&s.cond, &s.mutex);
    }
    s.value--;  // Thread can go further
    pthread_mutex_unlock(&s.mutex); // Leaving critical section
}

void signalSemaphore(Semaphore& s) {
    // Entering critical section, because we will adjust "value"
    pthread_mutex_lock(&s.mutex);
    s.value++;
    // Wake up one waiting thread
    pthread_cond_signal(&s.cond);
    pthread_mutex_unlock(&s.mutex);
}
