#pragma once


#include "../include/dap/network.h"
#include "logger.h"
#include <dap/io.h>

class RDebugClient {
private:
	std::shared_ptr<dap::ReaderWriter> client = nullptr;
	int remotePort = 0;
	Logger* logger = Logger::getInstance();

	RDebugClient();
public:
	static RDebugClient* createRDebugClient(int remotePort);

	~RDebugClient();

	bool connect(int remotePort);
	bool disconnect();
	bool isOpen();

	bool start();
	bool finish();

};