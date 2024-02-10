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

