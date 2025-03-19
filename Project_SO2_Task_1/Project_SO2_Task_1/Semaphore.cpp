#include "Semaphore.h"

void initSemaphore(Semaphore& s, int initValue) {
    s.value = initValue;
    pthread_mutex_init(&s.mutex, nullptr);
    pthread_cond_init(&s.cond, nullptr);
}

void waitSemaphore(Semaphore& s) {
    pthread_mutex_lock(&s.mutex);
    // If value <= 0, wait
    while (s.value <= 0) {
        pthread_cond_wait(&s.cond, &s.mutex);
    }
    s.value--;
    pthread_mutex_unlock(&s.mutex);
}

void signalSemaphore(Semaphore& s) {
    pthread_mutex_lock(&s.mutex);
    s.value++;
    // Wake up one waiting thread
    pthread_cond_signal(&s.cond);
    pthread_mutex_unlock(&s.mutex);
}
