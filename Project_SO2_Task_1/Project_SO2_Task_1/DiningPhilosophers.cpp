#include "DiningPhilosophers.h"

#include <iostream>
#include <cstdlib>
#include "CompatPthread.h"
#include <thread>
#include <time.h>
#include <chrono>

using namespace std;

static int N = 0;	// Number of philosophers
static PhilosopherState *stateArray = nullptr;	// Dynamic array of states per each philosopher
static Semaphore* semPhilosopher = nullptr;	// Semaphores per philosopher
static pthread_mutex_t stateMutex;	// Protects stateArray; global mutex

// Helpewr functions to get index of left/right neighbor
static inline int left(int i) {
	return (i + N - 1) % N;
}

static inline int right(int i) {
	return (i + 1) % N;
}

// Print a message about current state
static void print_state(int i, const char* msg) {
	cout << "Philosopher number " << i << " is " << msg << endl;
}

// Check if philosopher i can start EATING
static void test(int i) {
	if (stateArray[i] == HUNGRY && stateArray[left(i)] != EATING && stateArray[right(i)] != EATING) {
		stateArray[i] = EATING;
		print_state(i, "EATING");
		signalSemaphore(semPhilosopher[i]);
	}
}

// Philosopher i tries to pick up chopsticks
static void pickup(int i) {
	pthread_mutex_lock(&stateMutex);	// Enter critical section

	stateArray[i] = HUNGRY;
	print_state(i, "HUNGRY");

	test(i);	// Possibly move from HUNGRY to EATING
	pthread_mutex_unlock(&stateMutex);

	// If not EATING yet - block here and wait for neighobors
	waitSemaphore(semPhilosopher[i]);
}

// Philosopher i puts down chopsticks
static void putdown(int i) {
	pthread_mutex_lock(&stateMutex);	// Enter critical section

	stateArray[i] = THINKING;
	print_state(i, "THINKING");

	// Check the neighbors
	test(left(i));
	test(right(i));

	pthread_mutex_unlock(&stateMutex);	// Unblock mutex
}

void initDiningPhilosophers(int n) {
	N = n;
	// Initialize arrays
	stateArray = new PhilosopherState[N];
	semPhilosopher = new Semaphore[N];
	// Initialize global mutex
	pthread_mutex_init(&stateMutex, nullptr);

	for (int i = 0; i < N; i++) {
		stateArray[i] = THINKING;
		initSemaphore(semPhilosopher[i], 0); // Each philosopher's semaphore starts at 0
	}

	// Seed random for sleep durations
	srand(static_cast<unsigned>(time(nullptr)));
}

void cleanupDiningPhilosophers() {
	// Destroy semaphores and free memory
	for (int i = 0; i < N; i++) {
		pthread_mutex_destroy(&semPhilosopher[i].mutex);
		pthread_cond_destroy(&semPhilosopher[i].cond);
	}
	pthread_mutex_destroy(&stateMutex);

	delete[] stateArray;
	delete[] semPhilosopher;
	stateArray = nullptr;
	semPhilosopher = nullptr;
	N = 0;
}

void* philosopherThread(void* arg) {
	int i = *(int*)arg;

	while (true) {
		// Philosopher i "THINKING"
		print_state(i, "THINKING");
		// Sleep for 1 - 2 seconds
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 + rand() % 1000));

		// Try to pick up chopsticks
		pickup(i);

		// If unblocked, philosopher is now EATING (as shown by test())
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 + rand() % 1000));

		// Put down chopsticks
		putdown(i);
	}

	return nullptr;
}