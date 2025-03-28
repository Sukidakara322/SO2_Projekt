#include <iostream>
#include "CompatPthread.h"
#include <cstdlib>
#include "DiningPhilosophers.h"

using namespace std;

int main(int argc, char* argv[]) {
	// Checking if user enetered number of philosophers as argument
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " <number_of_philosophers>" << endl;
		return 1;
	}

	// Converting argument from char to integer
	int n = std::atoi(argv[1]);
	if (n < 1) {
		cerr << "Number of philosophers must be >= 1" << endl;
		return 1;
	}

	// Initialize data structures of dining philosopher
	initDiningPhilosophers(n);

	pthread_t* threads = new pthread_t[n]; // Stores identifiers of philosopher's threads
	int* ids = new int[n]; // Indexes of philosophers

	// Create philosopher's threads
	for (int i = 0; i < n; i++) {
		ids[i] = i;
		pthread_create(&threads[i], nullptr, philosopherThread, &ids[i]);
	}

	// Join threads (though they run indefinitely)
	for (int i = 0; i < n; i++) {
		pthread_join(threads[i], nullptr);
	}

	// Cleanup
	cleanupDiningPhilosophers();
	delete[] threads;
	delete[] ids;

	return 0;
}