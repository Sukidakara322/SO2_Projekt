#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <pthread.h>

// Simple semaphore structure
typedef struct {
    int value;  // Current semaphore value
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Semaphore;

// Initialize the semaphore with a starting value
void initSemaphore(Semaphore& s, int initValue);

// "Wait" (P) operation. Blocks if the semaphore value is 0
void waitSemaphore(Semaphore& s);

// "Signal" (V) operation. Increments the semaphore and unblocks one waiter
void signalSemaphore(Semaphore& s);

#endif
