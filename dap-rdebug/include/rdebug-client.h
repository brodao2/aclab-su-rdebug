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

#pragma once

#include "../include/dap/network.h"
#include "logger.h"
#include <dap/io.h>

struct Breakpoint {
	std::string source = "";
	int line = 0;
	std::string index = "";

	Breakpoint(std::string source, int line, std::string index) {
		this->source = source;
		this->line = line;
		this->index = index;
	}
};

struct Thread {
	int id = 0;
	std::string name = "";

	Thread(int id, std::string name) {
		this->id = id;
		this->name = name;
	}
};

struct Frame {
	int id = 0;
	std::string source = "";
	int line = 0;
	bool current = false;

	Frame(int id, std::string source, int line, bool current) {
		this->id = id;
		this->source = source;
		this->line = line;
		this->current = current;
	}
};

struct Variable {
	std::string name = "";
	int namedVariables = 0;
	int indexedVariables = 0;
	std::vector<Variable*> variables;
	int referenceId = 0;
	std::string type;
	std::string value;
	std::string object_id;
	std::string evaluateName;

	Variable(std::string name) {
		this->name = name;
	}

	~Variable() {
		for (size_t i = 0; i < this->variables.size(); i++) {
			//delete this->variables.at(i);
		}
		this->variables.clear();
	}
};

struct Scope {
	std::string command = "";
	std::string name = "";
	int namedVariables = 0;
	int indexedVariables = 0;
	std::vector<Variable*> variables;
	int referenceId = 0;

	Scope(std::string command, std::string name) {
		this->name = name;
		this->command = command;
	}

	~Scope() {
		for (size_t i = 0; i < this->variables.size(); i++) {
			//delete this->variables.at(i);
		}
		this->variables.clear();
	}
};

class RDebugClient {
private:
	std::shared_ptr<dap::ReaderWriter> client = nullptr;
	int remotePort = 0;
	Logger* logger = Logger::getInstance();
	std::map<std::string, std::vector<Breakpoint*>*> breakpointsMap;
	std::map<int, Scope*> scopeMap;

	RDebugClient();
protected:
	bool send(std::string command);
	std::string sendAndWait(std::string command);

	bool addScopeMap(Scope* scope);
	bool removeScopeMap(Scope* scope);

public:
	static RDebugClient* createRDebugClient(int remotePort);

	~RDebugClient();

	Breakpoint* waitBreakpoint();

	bool connect(int remotePort);
	bool disconnect();
	bool isOpen();
	bool quit();
	bool start();
	bool continue_();
	bool interrupt();
	bool next();
	bool stepOut();

	bool addBreakpoint(std::string source, int line, std::string condition);
	bool removeBreakpoint(std::string source);
	bool removeBreakpoint2(std::string indexes);
	bool setEnableBreakpoint(std::string source, int line, bool enable);;
	bool setEnableBreakpoint2(std::string index, bool enable);
	void updateBreakpointsMap();
	bool threads(std::vector<Thread>& threadList);
	bool where(std::vector<Frame>& frameList);
	bool scopes(std::vector<Scope*>& scopeList);

	bool getVariables(Scope* scope);
	Scope* findScopeMap(std::string name);
	Scope* findScopeMap(int refereceId);
};