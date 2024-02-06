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
	void endChrono(std::string label);

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