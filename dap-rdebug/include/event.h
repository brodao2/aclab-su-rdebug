/*
Copyright 2024 Alan Cândido (brodao@gmail.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http: //www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <mutex>

class Event {
public:
	// wait() blocks until the event is fired.
	void wait();

	// wait(int timeout) blocks until the event is fired with timeout (seconds).
	// return true if fired
	bool wait(int timeout);

	// fire() sets signals the event, and unblocks any calls to wait().
	void fire();

private:
	std::mutex mutex;
	std::condition_variable cv;
	bool fired = false;
};

