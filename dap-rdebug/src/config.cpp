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
