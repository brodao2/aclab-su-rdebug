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

	auto breakPoint_i = this->breakpointsMap.begin();
	while (breakPoint_i != this->breakpointsMap.end()) {
		this->removeBreakpoint(breakPoint_i->first);
		breakPoint_i = this->breakpointsMap.begin();
	}
	this->breakpointsMap.clear();

	auto scopeMap_i = this->scopeMap.begin();
	while (scopeMap_i != this->scopeMap.end()) {
		this->removeScopeMap(scopeMap_i->second);
		scopeMap_i = this->scopeMap.begin();
	}
	this->scopeMap.clear();

	logger->endTrace("RDebugClient::~RDebugClient");
}

bool RDebugClient::connect(int remotePort) {
	logger->startTrace("RDebugClient::connect");
	bool result = true;

	if (this->client) {
		logger->error("RDebug client already connected");
		result = false;
	} else {
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
		this->quit();
		this->client->close();
	}

	logger->endTrace("RDebugClient::disconnect");
	return true;
}

const std::string_view CMD_START = "start";
const std::string_view CMD_FINISH = "finish";
const std::string_view CMD_CONTINUE = "cont";
const std::string_view CMD_NEXT = "next";
const std::string_view CMD_PAUSE = "pause ???";
const std::string_view CMD_INTERRUPT = "interrupt";
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
const std::string_view CMD_WHERE = "where";
const std::string_view CMD_EVAL = "eval";
const std::string_view CMD_INSPECT = "var inspect";
const std::string_view CMD_VAR_GLOBAL = "var global";
const std::string_view CMD_VAR_INSTANCE = "var instance";
const std::string_view CMD_VAR_LOCAL = "var local";

const std::regex ADD_BREAKPOINT_RESPONSE_RE(R"(<breakpointAdded no=\"(\d+)\"[^>]+>)", std::regex_constants::icase);
const std::regex THREAD_LINE_RESPONSE_RE(R"(<thread id=\"(\d+)\" status=\"(\w+)\"[^>]+>)", std::regex_constants::icase);
const std::regex FRAME_RESPONSE_RE(R"(<frame no\s*=\s*\"(\d+)\"\s*file\s*=\s*\"([^\"]*)\"\s*line\s*=\s*\"(\d+)\"\s*(currents*=s*"yes")?\s*\/>)", std::regex_constants::icase);
const std::regex VARIABLE_RESPONSE_RE(R"(<variable\s+name=\"(\w+)\"\s+kind=\"([^\"]+)\"\s+value=\"([^\"]+)\"\s+type=\"([^\"]+)\"\s+hasChildren=\"(true|false)\"\s+objectId=\"(0x[0-9a-fA-F]+)\"\s*\/>)", std::regex_constants::icase);

const std::regex HTML_ENTITE_RE(R"(#&[^;]+;)", std::regex_constants::icase);

bool RDebugClient::start() {
	logger->startTrace("RDebugClient::start");

	this->updateBreakpointsMap();

	const bool result = this->send(CMD_START.data());

	logger->endTrace("RDebugClient::start");
	return result;
}

bool RDebugClient::quit() {
	logger->startTrace("RDebugClient::quit");

	const bool result = this->send(CMD_QUIT.data());

	logger->endTrace("RDebugClient::quit");
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
			buffer[read - 2] = '\0'; //elimina \r\n
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
		auto breakpointsMap_i = this->breakpointsMap.find(source);

		if (breakpointsMap_i == this->breakpointsMap.end()) {
			this->breakpointsMap.emplace(source, new std::vector<Breakpoint*>());
			breakpointsMap_i = this->breakpointsMap.find(source);
		}

		breakpointsMap_i->second->push_back(new Breakpoint(source, line, index_s));

		this->setEnableBreakpoint2(index_s, true);
	}

	logger->endTrace("RDebugClient::addBreakpoint");
	return response.length() > 0;
}

bool RDebugClient::setEnableBreakpoint(std::string source, int line, bool enable) {
	logger->startTrace("RDebugClient::setEnableBreakpoint");
	bool result = false;

	auto breakpointsMap_i = this->breakpointsMap.find(source);
	if (breakpointsMap_i != this->breakpointsMap.end()) {
		std::vector<Breakpoint*>* breakpoints = breakpointsMap_i->second;

		for (size_t i = 0; i < breakpoints->size(); i++) {
			Breakpoint* breakPoint = breakpoints->at(i);

			if ((breakPoint->line == line)) {
				result = this->setEnableBreakpoint2(breakPoint->index, enable);
				break;
			}
		}
	}

	logger->endTrace("RDebugClient::setEnableBreakpoint");
	return result;
}

bool RDebugClient::setEnableBreakpoint2(std::string index, bool enable) {
	logger->startTrace("RDebugClient::setEnableBreakpoint");

	//^(en|dis)(?:able)?\\s+breakpoints((?:\\s+\\d+)+)$
	std::string command;
	command.append(enable ? CMD_ENABLE_BREAKPOINTS.data() : CMD_DISABLE_BREAKPOINTS.data())
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

	auto breakpointsMap_i = this->breakpointsMap.find(source);
	if (breakpointsMap_i != this->breakpointsMap.end()) {
		std::vector<Breakpoint*>* breakpoints = breakpointsMap_i->second;

		for (size_t i = 0; i < breakpoints->size(); i++) {
			Breakpoint* breakPoint = breakpoints->at(i);

			indexes.append(" ").append(breakPoint->index);

			delete breakPoint;
		}
	}

	this->breakpointsMap.erase(source);

	if (this->isOpen() && !indexes.empty()) {
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

	for (auto breakPoint_i = this->breakpointsMap.begin(); breakPoint_i != this->breakpointsMap.end(); breakPoint_i++) {
		//Breakpoint* breakpoint = breakPoint_i->second;

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
		for (size_t i = 1; i < lines.size() - 1; i++) {
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

Breakpoint* RDebugClient::waitBreakpoint() {
	char* buffer = (char*)std::malloc(1024);
	int read = this->client->read(buffer, 1024);
	if (read > 2) {
		buffer[read - 2] = '\0'; //elimina \r\n
		std::string text(buffer);

		if (text != "") {
			//<breakpoint file="p:/GitHub/sketchup-ruby-api-tutorials/examples/01_hello_cube/ex_hello_cube/main.rb" line="10" threadId="1" />
			auto breakpointsMap_i = this->breakpointsMap.begin(); // tratar / para \\ find("p:/GitHub/sketchup-ruby-api-tutorials/examples/01_hello_cube/ex_hello_cube/main.rb");
			if (breakpointsMap_i != this->breakpointsMap.end()) {
				std::vector<Breakpoint*>* breakpoints = breakpointsMap_i->second;

				for (size_t i = 0; i < breakpoints->size(); i++) {
					Breakpoint* breakPoint = breakpoints->at(i);

					if ((breakPoint->line == 10)) {
						return breakPoint;
					}
				}
			}
		}
		return nullptr;

	}

	return nullptr;
}

bool RDebugClient::where(std::vector<Frame>& frameList) {
	logger->startTrace("RDebugClient::where");
	bool result = false;

	//^(?:w(?:here)?|bt|backtrace)$;
	std::string command;
	command.append(CMD_WHERE.data());

	//<frames>
	//  <frame no=\"" << (index + 1) << "\" file=\"" << escapeXml(frame.file) << "\" line=\"" << frame.line << "\" " current=\"yes\"" />;
	//</frames>"
	std::string response = this->sendAndWait(command);
	std::vector<std::string> lines = splitResponse(response);

	if (!lines.empty()) {
		for (size_t i = 1; i < lines.size() - 1; i++) {
			std::string line = lines[i];
			std::smatch match;

			if (std::regex_match(line, match, FRAME_RESPONSE_RE)) {
				std::string index_s = match[1];
				std::string file = match[2];
				int lineNumber = std::atoi(match[3].str().c_str());
				bool current = match.length() > 3;
				//<frame no="2" file="P:/GitHub/sketchup-ruby-api-tutorials/examples/01_hello_cube/ex_hello_cube/main.rb" line="28" />
				int index = std::stoi(index_s);
				frameList.emplace_back(Frame(index, file, lineNumber, current));
			}
		}
	}

	logger->endTrace("RDebugClient::threads");
	return frameList.size() > 0;
}

bool RDebugClient::scopes(std::vector<Scope*>& scopeList) {
	logger->startTrace("RDebugClient::scopes");

	Scope* local = this->findScopeMap("local");
	if (local == nullptr) {
		local = new Scope(CMD_VAR_LOCAL.data(), "local");
		local->namedVariables = 1;
		this->addScopeMap(local);
	}
	scopeList.emplace_back(local);

	//Scope instance(CMD_VAR_INSTANCE.data(), "instance");
	//this->getVariables(instance);
	//scopeList.emplace_back(instance);

	Scope* global = this->findScopeMap("global");
	if (global == nullptr) {
		global = new Scope(CMD_VAR_GLOBAL.data(), "global");
		global->namedVariables = 1;
		this->addScopeMap(global);
	}
	scopeList.emplace_back(global);

	logger->endTrace("RDebugClient::scopes");
	return scopeList.size() > 0;
}

bool RDebugClient::getVariables(Scope* scope) {
	logger->startTrace("RDebugClient::getVariables");
	bool result = false;

	//global  : "^v(?:ar)?\\s+g(?:lobal)?$
	//instance: "^v(?:ar)?\\s+i(?:nstance)?\\s+(?:0x)?([\\da-f]+)$"
	//local   : "^v(?:ar)?\\s+l(?:ocal)?$"
	std::string command = scope->command;

	//<variables>
	//  <variable name=\"" << escapeXml(var.name) << "\" kind=\"" << kind << "\" value=\"" << escapeXml(var.value) << "\" type=\"" << var.type << "\" hasChildren=\"" << (var.has_children ? "true" : "false") << "\" objectId=\"0x" << std::hex << var.object_id << "\" />";
	//</variables>"
	std::string response = this->sendAndWait(command);
	std::vector<std::string> lines = splitResponse(response);

	if (!lines.empty()) {
		scope->variables.clear();

		for (size_t i = 1; i < lines.size() - 1; i++) {
			std::string line = lines[i];
			std::smatch match;

			if (std::regex_match(line, match, VARIABLE_RESPONSE_RE)) {
				Variable* variable = new Variable(match[1]);

				//variable->name = match[1];
				variable->type = match[4];
				variable->value = this->cleanHtmlEntities(match[3]);
				variable->namedVariables= match[5] == "false" ? 0 : 1;
				variable->object_id = match[6];
				variable->evaluateName = variable->name;

				scope->variables.emplace_back(variable);
				//scope->namedVariables++;
			}
		}
	}

	logger->endTrace("RDebugClient::getVariables");
	return scope->variables.size() > 0;
}

std::string RDebugClient::cleanHtmlEntities(std::string value) {

	return std::regex_replace(value, HTML_ENTITE_RE, "");
}

bool RDebugClient::addScopeMap(Scope* scope) {
	static int referenceId = 0;

	scope->referenceId = ++referenceId;
	this->scopeMap.emplace(referenceId, scope);

	return true;
}

bool RDebugClient::removeScopeMap(Scope* scope) {
	this->scopeMap.erase(scope->referenceId);

	return true;
}

Scope* RDebugClient::findScopeMap(std::string name) {
	for (auto variableMap_i = this->scopeMap.begin(); variableMap_i != this->scopeMap.end(); variableMap_i++) {
		Scope* scope = variableMap_i->second;

		if (scope->name == name) {
			return scope;
		}
	}

	return nullptr;
}

Scope* RDebugClient::findScopeMap(int referenceId) {
	for (auto variableMap_i = this->scopeMap.begin(); variableMap_i != this->scopeMap.end(); variableMap_i++) {
		Scope* scope = variableMap_i->second;

		if (scope->referenceId == referenceId) {
			return scope;
		}
	}

	return nullptr;
}

bool RDebugClient::next() {
	logger->startTrace("RDebugClient::next");
	bool result = false;

	//^n(?:ext)
	std::string command;
	command.append(CMD_NEXT.data());

	//<suspended file = "P:/GitHub/sketchup-ruby-api-tutorials/examples/01_hello_cube/ex_hello_cube/main.rb" line = "11" threadId = "1" frames = "1" / >
	std::string r = this->sendAndWait(command);

	logger->endTrace("RDebugClient::send");
	return true;
}

bool RDebugClient::stepOut() {
	logger->startTrace("RDebugClient::stepOut");
	bool result = false;
	
	std::string command;
	command.append(CMD_FINISH.data());

	//<suspended file = "P:/GitHub/sketchup-ruby-api-tutorials/examples/01_hello_cube/ex_hello_cube/main.rb" line = "11" threadId = "1" frames = "1" / >
	std::string r = this->sendAndWait(command);

	logger->endTrace("RDebugClient::stepOut");
	return true;
}

bool RDebugClient::interrupt() {
	logger->startTrace("RDebugClient::interrupt");
	bool result = false;

	//(?:pause(?:\\s+(\\d+))?|i(?:nterrupt)?)$
	std::string command;
	command.append(CMD_INTERRUPT.data());

	//<suspended file = "P:/GitHub/sketchup-ruby-api-tutorials/examples/01_hello_cube/ex_hello_cube/main.rb" line = "11" threadId = "1" frames = "1" / >
	std::string r = this->sendAndWait(command);

	logger->endTrace("RDebugClient::interrupt");
	return true;
}
