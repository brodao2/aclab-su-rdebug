#pragma once


#include "../include/dap/network.h"
#include "logger.h"
#include <dap/io.h>

struct Breakpoint {
	std::string source;
	int line;
	std::string index;

	Breakpoint(std::string source, int line, std::string index) {
		this->source = source;
		this->line = line;
		this->index = index;
	}
};

struct Thread {
	// Unique identifier for the thread.
	int id;
	// The name of the thread.
	std::string name;

	Thread(int id, std::string name) {
		this->id = id;
		this->name = name;
	}
};


class RDebugClient {
private:
	std::shared_ptr<dap::ReaderWriter> client = nullptr;
	int remotePort = 0;
	Logger* logger = Logger::getInstance();
	std::map<std::string, std::vector<Breakpoint*>*> breakpointsMap;

	RDebugClient();
protected:
	bool send(std::string command);
	std::string sendAndWait(std::string command);

public:
	static RDebugClient* createRDebugClient(int remotePort);

	~RDebugClient();

	bool connect(int remotePort);
	bool disconnect();
	bool isOpen();


	bool start();
	bool finish();
	bool continue_();

	bool addBreakpoint(std::string source, int line, std::string condition);
	bool removeBreakpoint(std::string source);
	bool removeBreakpoint2(std::string indexes);
	bool setEnableBreakpoint(std::string source, int line, bool enable);;
	bool setEnableBreakpoint2(std::string index, bool enable);
	void updateBreakpointsMap();
	bool threads(std::vector<Thread>& threadList);
};