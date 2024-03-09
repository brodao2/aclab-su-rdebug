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

#include "config.h"
#include "utils.h"

Config::Config() {
	//
}

Config::~Config() {
	//
}

void Config::setLogLevel(spdlog::level::level_enum level) {
	this->logLevel = level;
}

void Config::setLogFile(std::string filename) {
	this->logFile = filename;
}

void Config::setRemotePort(int port) {
	this->remotePort = port;
}

void Config::setExtension(std::string extensionPath) {
	this->extension = extensionPath;
}

spdlog::level::level_enum Config::getLogLevel() {
	return this->logLevel;
}

std::string Config::getLogFile() {
	return this->logFile;
}

int Config::getRemotePort() {
	return this->remotePort;
}

int Config::getDebugAdapterPort() {
	return this->remotePort + 1;
}

std::string Config::getExtension() {
	return this->extension;
}

bool Config::isWaitAttach() {
	return this->waitAttach;
}
