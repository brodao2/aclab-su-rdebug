#pragma once

#include <string>
#include <vector>
#include "spdlog/spdlog.h"

class Config {
private:

	Config();
	~Config();

	spdlog::level::level_enum logLevel = spdlog::level::level_enum::info;
	std::string logFile = "";
	int remotePort = 1234; //RDebug default port
	std::string extension = "";
	std::string sketchupProgram = "";
	std::vector<std::string> sketchupArguments = {};

public:
	static Config* getInstance()
	{
		static Config instance;

		return &instance;
	}

	void setLogLevel(spdlog::level::level_enum level);
	void setLogFile(std::string filename);
	void setRemotePort(int port);
	void setExtension(std::string extensionPath);
	void setSketchupProgram(std::string executablePath);
	void setSketchupArguments(std::vector<std::string> arguments);

	spdlog::level::level_enum getLogLevel();
	std::string getLogFile();
	int getRemotePort();
	std::string getExtension();
	std::string getSketchupProgram();
	std::vector<std::string> getSketchupArguments();

	int getServerPort();

};