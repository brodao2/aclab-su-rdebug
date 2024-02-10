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

void Config::setSketchupProgram(std::string executablePath) {
	this->sketchupProgram = executablePath;
}

void Config::setSketchupArguments(std::vector<std::string> arguments) {
	this->sketchupArguments = arguments;
}

spdlog::level::level_enum Config::getLogLevel() {
	return this->logLevel;
}

std::string Config::getLogFile() {
	return this->logFile;
}

int Config::_getRemotePort() {
	return this->remotePort;
}

int Config::getDebugAdapterPort() {
	return this->remotePort + 1;
}

std::string Config::getExtension() {
	return this->extension;
}

std::string Config::getSketchupProgram() {
	return this->sketchupProgram;
}

std::vector<std::string> Config::getSketchupArguments() {
	return this->sketchupArguments;
}
