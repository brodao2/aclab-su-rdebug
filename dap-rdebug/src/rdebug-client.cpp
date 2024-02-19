#include "rdebug-client.h"
#include <chrono>
#include <thread>
#include <streambuf>
#include <regex>
#include <utils.h>
#include <iostream>

const std::regex RESPONSE_LINE_RE("(<[^>]+>)");

std::vector<std::string> splitResponse(std::string text) {
	std::smatch matches;
	std::vector<std::string> result;

	while (std::regex_search(text, matches, RESPONSE_LINE_RE)) {
		result.emplace_back(matches[1]);
		
		text = matches.suffix();
	}

	return result;
}

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

	for (auto breakPoint_i = this->breakpointsMap.begin(); breakPoint_i != this->breakpointsMap.end(); breakPoint_i++)
	{
		delete breakPoint_i->second;
	}
	this->breakpointsMap.clear();

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

const std::string_view CMD_START = "start";
const std::string_view CMD_FINISH = "finish";

const std::string_view CMD_CONTINUE = "cont";
const std::string_view CMD_NEXT = "next";
const std::string_view CMD_PAUSE = "pause";
const std::string_view CMD_QUIT = "quit";
const std::string_view CMD_STEP = "step";
const std::string_view CMD_ADD_BREAKPOINT = "break";
const std::string_view CMD_BREAKPOINTS = "info break";
const std::string_view CMD_CONDITION = "condition";
const std::string_view CMD_DELETE_BREAKPOINT = "delete";
const std::string_view CMD_INFO_BREAKPOINT = "info break";
const std::string_view CMD_ENABLE_BREAKPOINTS = "enable breakpoints";
const std::string_view CMD_DISABLE_BREAKPOINTS = "disable breakpoints";
const std::string_view CMD_FRAME = "frame";
const std::string_view CMD_THREAD_LIST = "thread list";
const std::string_view CMD_WHERE = "where backtrace";
const std::string_view CMD_EVAL = "eval";
const std::string_view CMD_INSPECT = "var inspect";
const std::string_view CMD_VAR_GLOBAL = "var global";
const std::string_view CMD_VAR_INSTANCE = "var instance";
const std::string_view CMD_VAR_LOCAL = "var local";

const std::regex ADD_BREAKPOINT_RESPONSE_RE("<breakpointAdded no=\"(\\d+)\"[^>]+>", std::regex_constants::icase);
const std::regex THREAD_LINE_RESPONSE_RE("<thread id=\"(\\d+)\" status=\"(\\w+)\"[^>]+>", std::regex_constants::icase);

bool RDebugClient::start() {
	logger->startTrace("RDebugClient::start");

	this->updateBreakpointsMap();

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

bool RDebugClient::continue_() {
	logger->startTrace("RDebugClient::continue");

	const bool result = this->send(CMD_CONTINUE.data());

	logger->endTrace("RDebugClient::continue");
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
		if (read > 2) {
			buffer[read-2] = '\0'; //elimina \r\n
			std::string text(buffer);
			return text;
		}		
	}

	return "";
}

bool RDebugClient::addBreakpoint(std::string source, int line, std::string condition) {
	logger->startTrace("RDebugClient::addBreakpoint");
	
	//^b(?:reak)?\\s+(.+?):(\\d+)(?:\\s+if\\s+(.+))?$;
	std::string command;
	command.append(CMD_ADD_BREAKPOINT.data())
		.append(" ")
		.append(source)
		.append(":")
		.append(std::to_string(line));

	if (condition.length() > 0) {
		command.append(" if ")
			.append(condition);
	}

	//"<breakpointAdded no=\"" << bp.index << "\" location=\"" << escapeXml(bp.file) << ':' << bp.line << "\" />";
	std::string response = this->sendAndWait(command);
	std::smatch match;

	if (std::regex_match(response, match, ADD_BREAKPOINT_RESPONSE_RE)) {
		std::string index_s = match[1];
		this->breakpointsMap.emplace(index_s, new Breakpoint(source, line));

		this->setEnableBreakpoint(index_s, true);
	}

	logger->endTrace("RDebugClient::addBreakpoint");
	return response.length() > 0;
}

bool RDebugClient::setEnableBreakpoint(std::string source, int line, bool enable) {
	logger->startTrace("RDebugClient::setEnableBreakpoint");
	bool result = false;

	for (auto breakPoint_i = this->breakpointsMap.begin(); breakPoint_i != this->breakpointsMap.end(); breakPoint_i++)
	{
		Breakpoint* breakpoint = breakPoint_i->second;

		if ((breakpoint->source == source) && (breakpoint->line == line)) {
			result = this->setEnableBreakpoint(breakPoint_i->first, enable);
			break;
		}
	}

	logger->endTrace("RDebugClient::setEnableBreakpoint");
	return result;
}

bool RDebugClient::setEnableBreakpoint(std::string index, bool enable) {
	logger->startTrace("RDebugClient::setEnableBreakpoint");

	//^(en|dis)(?:able)?\\s+breakpoints((?:\\s+\\d+)+)$
	std::string command;
	command.append(enable?CMD_ENABLE_BREAKPOINTS.data(): CMD_DISABLE_BREAKPOINTS.data())
		.append(" ")
		.append(index);

	//"<breakpoint" << (enable ? "Enabled" : "Disabled") << " bp_id=\"" << index << "\" />
	std::string response = this->sendAndWait(command);

	logger->endTrace("RDebugClient::setEnableBreakpoint");
	return response.length() > 0;
}

bool RDebugClient::removeBreakpoint(std::string source) {
	bool result = true;
	std::string indexes = "";

	for (auto breakPoint_i = this->breakpointsMap.begin(); breakPoint_i != this->breakpointsMap.end(); breakPoint_i++)
	{
		Breakpoint* breakpoint = breakPoint_i->second;

		if ((breakpoint->source == source)) {
			result = this->setEnableBreakpoint(breakPoint_i->first, false);
			indexes.append(" ").append(breakPoint_i->first);

			delete breakPoint_i->second;
			this->breakpointsMap.erase(breakPoint_i->first);
		}
	}

	if (!indexes.empty()) {
		result = this->removeBreakpoint(indexes);
	}

	return result;
}

bool RDebugClient::removeBreakpoint2(std::string indexes) {
	logger->startTrace("RDebugClient::removeBreakpoint");

	//^del(?:ete)?(?:\\s+(\\d+))?$
	std::string command;
	command.append(CMD_DELETE_BREAKPOINT.data())
		.append(" ")
		.append(indexes);

	// "<breakpointDeleted no=\"" << index << "\" />";
	std::string response = this->sendAndWait(command);

	logger->endTrace("RDebugClient::removeBreakpoint");
	return response.length() > 0;
}

void RDebugClient::updateBreakpointsMap() {
	logger->startTrace("RDebugClient::updateBreakpointsMap");
	bool result = false;

	//^(?:info\\s*)?b(?:reak)?$
	std::string command;
	command.append(CMD_INFO_BREAKPOINT.data());

	//<breakpoints>";
	// <breakpoint n=\"" << bp.index << "\" file=\"" << escapeXml(bp.file) << "\" line=\"" << bp.line << "\" />
	//</breakpoints>";
	std::string response = this->sendAndWait(command);
	std::vector<std::string> lines = splitResponse(response);

	for (auto breakPoint_i = this->breakpointsMap.begin(); breakPoint_i != this->breakpointsMap.end(); breakPoint_i++)
	{
		Breakpoint* breakpoint = breakPoint_i->second;

		//if ((breakpoint->source == source) && (breakpoint->line == line)) {
		//	result = this->setEnableBreakpoint(breakPoint_i->first, enable);
		//	break;
		//}
	}

	logger->endTrace("RDebugClient::updateBreakpointsMap");
}

bool RDebugClient::threads(std::vector<Thread>& threadList) {
	logger->startTrace("RDebugClient::threads");
	bool result = false;

	//^th(?:read)?\\s+l(?:ist)?$
	std::string command;
	command.append(CMD_THREAD_LIST.data());

	//<threads>
	// <thread id=\"1\" status=\"run\" />
	// </threads>"
	std::string response = this->sendAndWait(command);
	std::vector<std::string> lines = splitResponse(response);

	if (!lines.empty()) {
		for (size_t i = 1; i < lines.size() - 1; i++)
		{
			std::string line = lines[i];
			std::smatch match;
			
			if (std::regex_match(line, match, THREAD_LINE_RESPONSE_RE)) {
				std::string index_s = match[1];
				int index = std::stoi(index_s);
				threadList.emplace_back(Thread(index, index_s));
			}
		}
	}

	logger->endTrace("RDebugClient::threads");
	return threadList.size() > 0;
}
