#include "rdebug-client.h"

RDebugClient::RDebugClient() {
	logger->startTrace("RDebugClient::RDebugClient");

	this->logger = Logger::getInstance();

	logger->endTrace("RDebugClient::RDebugClient");
}

RDebugClient::~RDebugClient() {
	logger->startTrace("RDebugClient::~RDebugClient");

	this->disconnect();

	logger->endTrace("RDebugClient::~RDebugClient");
}

bool RDebugClient::connect(int remotePort) {
	logger->startTrace("RDebugClient::connect");
	bool result = true;

	if (this->client) {
		logger->error("RDebug client already connected");
		result = false;
	}
	else {
		this->client = dap::net::connect("localhost", remotePort);

		if (!this->client) {
			logger->error("Couldn't connect to server");
			result = false;
		}
	}

	logger->endTrace("RDebugClient::connect");
	return result;
}

bool RDebugClient::disconnect() {
	logger->startTrace("RDebugClient::disconnect");

	if (this->client) {
		this->client->close();
	}

	logger->endTrace("RDebugClient::disconnect");
	return true;
}
