#ifndef DININGPHILOSOPHERS_H
#define DININGPHILOSOPHERS_H

#include "Semaphore.h"

// Possible states a philosopher can be in
enum PhilosopherState {
    THINKING,
    HUNGRY,
    EATING
};

// Initialize all data structures for the Dining Philosophers problem
void initDiningPhilosophers(int n);

// Cleanup any allocated resources
void cleanupDiningPhilosophers();


// The thread function for each philosopher
void* philosopherThread(void* arg);

#endif
