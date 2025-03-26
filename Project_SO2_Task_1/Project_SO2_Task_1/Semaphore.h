#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "CompatPthread.h"

// Semaphore structure
typedef struct {
    int value;  // Current semaphore value
    pthread_mutex_t mutex; // Mutex to protect semaphore's inner value
    pthread_cond_t cond;    // Conditional var for threads to wait
} Semaphore;

// Initialize the semaphore with a starting value
void initSemaphore(Semaphore& s, int initValue);

// "Wait" (P) operation, blocks if the semaphore value is 0
void waitSemaphore(Semaphore& s);

// "Signal" (V) operation, increments the semaphore and unblocks one waiter
void signalSemaphore(Semaphore& s);

#endif
