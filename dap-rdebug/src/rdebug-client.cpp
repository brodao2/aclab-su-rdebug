#include "rdebug-client.h"
#include <chrono>
#include <thread>
#include <streambuf>

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
constexpr std::string_view CMD_BREAKPOINTS = "info break";
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

	const bool result = this->send(CMD_START.data());

	logger->endTrace("RDebugClient::start");
	return result;
}

bool RDebugClient::finish() {
	logger->startTrace("RDebugClient::finish");

	const bool result = this->send(CMD_START.data());

	logger->endTrace("RDebugClient::finish");
	return result;
}

bool RDebugClient::send(std::string command) {
	std::string sendBuffer = command + "\r\n";

	const bool result = this->client->write(sendBuffer.c_str(), sendBuffer.length());

	return result;
}

std::string RDebugClient::sendAndWait(std::string command) {
	const bool result = this->send(command.c_str());

	if (result) {
		char* buffer = (char*)std::malloc(1024);
		int read = this->client->read(buffer, 1024);
		buffer[read + 1] = '\0';
		std::string text(buffer);

		return text;
	}

	return "";
}

bool RDebugClient::addBreakpoint(std::string source, int line) {
	logger->startTrace("RDebugClient::addBreakpoint");
	
	//breakpoint source:line
	std::string command;
	command.append(CMD_ADD_BREAKPOINT.data())
		.append(" ")
		.append(source)
		.append(":")
		.append(std::to_string(line));

	//"<breakpointAdded no=\"" << bp.index << "\" location=\"" << escapeXml(bp.file) << ':' << bp.line << "\" />";
	std::string response = this->sendAndWait(command);

	logger->endTrace("RDebugClient::finish");
	return response.length() > 0;
}

bool RDebugClient::removeBreakpoint(std::string source, int line) {
	logger->startTrace("RDebugClient::removeBreakpoint");

	int index = 0;
	//delete index
	std::string command;
	command.append(CMD_DELETE_BREAKPOINT.data())
		.append(" ")
		.append(std::to_string(index));

	// "<breakpointDeleted no=\"" << index << "\" />";
	std::string response = this->sendAndWait(command);

	logger->endTrace("RDebugClient::finish");
	return response.length() > 0;
}
