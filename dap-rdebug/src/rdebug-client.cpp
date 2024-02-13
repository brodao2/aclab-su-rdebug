#include "rdebug-client.h"
#include <chrono>
#include <thread>

RDebugClient* RDebugClient::createRDebugClient(int remotePort) {
	RDebugClient* client = new RDebugClient();
	int tryes = 10;

	do
	if (!client->connect(remotePort)) {
		tryes--;
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}

	while ((!client->isOpen()) && (tryes > 0));

	if (!client->isOpen()) {
		delete client;
		client = nullptr;
	}
	return client;
}

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

bool RDebugClient::isOpen() {
	logger->startTrace("RDebugClient::isOpen");
	bool result = false;

	if (this->client) {
		result = this->client->isOpen();
	}

	logger->endTrace("RDebugClient::isOpen");
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

constexpr std::string_view CMD_START = "start";
constexpr std::string_view CMD_FINISH = "finish";

constexpr std::string_view CMD_CONTINUE = "cont";
constexpr std::string_view CMD_NEXT = "next";
constexpr std::string_view CMD_PAUSE = "pause";
constexpr std::string_view CMD_QUIT = "quit";
constexpr std::string_view CMD_STEP = "step";
constexpr std::string_view CMD_ADD_BREAKPOINT = "break";
constexpr std::string_view CMD_BREAKPOINTS = "infro break";
constexpr std::string_view CMD_CONDITION = "condition";
constexpr std::string_view CMD_DELETE_BREAKPOINT = "delete";
constexpr std::string_view CMD_ENABLE_BREAKPOINT = "enable breakpoint";
constexpr std::string_view CMD_DISENABLE_BREAKPOINT = "disable breakpoint";
constexpr std::string_view CMD_FRAME = "frame";
constexpr std::string_view CMD_THREAD_LIST = "thread list";
constexpr std::string_view CMD_WHERE = "where backtrace";
constexpr std::string_view CMD_EVAL = "eval";
constexpr std::string_view CMD_INSPECT = "var inspect";
constexpr std::string_view CMD_VAR_GLOBAL = "var global";
constexpr std::string_view CMD_VAR_INSTANCE = "var instance";
constexpr std::string_view CMD_VAR_LOCAL = "var local";


bool RDebugClient::start() {
	logger->startTrace("RDebugClient::start");

	const bool result = this->client->write(CMD_START.data(), CMD_START.length());

	logger->endTrace("RDebugClient::start");
	return result;
}

bool RDebugClient::finish() {
	logger->startTrace("RDebugClient::finish");

	const bool result = this->client->write(CMD_START.data(), CMD_START.length());

	logger->endTrace("RDebugClient::finish");
	return result;
}

