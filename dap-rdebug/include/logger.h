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

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/stopwatch.h>
#include <map>

class Logger {
private:
	//std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console = nullptr;
	//std::shared_ptr<spdlog::sinks::basic_file_sink_mt> fileLog = nullptr;
	//spdlog::logger* allLoggers = nullptr;
	
	std::shared_ptr<spdlog::logger> console = nullptr;
	std::shared_ptr<spdlog::logger> fileLog = nullptr;
	std::map<std::string, spdlog::stopwatch*> chronoMap;

	Logger();
	~Logger();

protected:
	void startChrono(std::string label);
	void chrono(std::string label);
	double endChrono(std::string label);

public:
	static Logger* getInstance()
	{
		static Logger instance;

		return &instance;
	}

	void startTrace(std::string label);
	void trace(std::string label, std::string message, ...);
	void endTrace(std::string label);

	void debug(std::string message, ...);
	void info(std::string message, ...);
	void warn(std::string message, ...);
	void error(std::string message, ...);
	void critical(std::string message, ...);
};