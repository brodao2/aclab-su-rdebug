#include "event.h"
#include <iostream>

void Event::wait() {
	std::unique_lock<std::mutex> lock(mutex);
	
	this->cv.wait(lock, [&] { return fired; });
}

//
bool Event::wait(int timeout) {
	std::unique_lock<std::mutex> lock(mutex);

	return cv.wait_for(lock, std::chrono::seconds(timeout), [&] { return this->fired; });
}

void Event::fire() {
	std::unique_lock<std::mutex> lock(mutex);
	std::cout << "event fired";
	this->fired = true;
	this->cv.notify_all();
}
