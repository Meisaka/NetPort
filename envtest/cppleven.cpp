
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<int> somevalue(0);

void wastetime(int howmuch) {
	for(int i = 0; i < howmuch; i++) {
		somevalue++;
	}
}

void wastetimereffingsomething(std::atomic<int>& thingthatisreffed, int howmuch) {
	for(int i = 0; i < howmuch; i++) {
		thingthatisreffed++;
	}
}

int main() {
	std::thread runa, rune;
	std::chrono::high_resolution_clock::time_point firstpointintime = std::chrono::high_resolution_clock::now();
	runa = std::thread(wastetime,3000);
	rune = std::thread(wastetimereffingsomething, std::ref(somevalue), 3000);

	runa.join();
	rune.join();

	std::chrono::high_resolution_clock::time_point secondpointintime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds theamountoftimeinnanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(secondpointintime - firstpointintime);
	std::cout << "The value of somevalue is equal to " << somevalue << std::endl;
	std::cout << "The amount of time taken was " << theamountoftimeinnanoseconds.count() << " nanoseconds." << std::endl;

	return 0;
}
