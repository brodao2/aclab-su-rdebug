#include "logger.h"
#include <string.h> 
#include "config.h"

/*
	trace = SPDLOG_LEVEL_TRACE,
	debug = SPDLOG_LEVEL_DEBUG,
	info = SPDLOG_LEVEL_INFO, <<<< default
	warn = SPDLOG_LEVEL_WARN,
	err = SPDLOG_LEVEL_ERROR,
	critical = SPDLOG_LEVEL_CRITICAL,
	off = SPDLOG_LEVEL_OFF,
*/

Logger::Logger() {
	Config* config = Config::getInstance();
	//spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
	//spdlog::set_pattern("[%H:%M:%S] [%n] [%-5l] [%6t] %v");
	//spdlog::set_pattern("[%H:%M:%S] [dap-rdebug] [%-5l] [%6t] %v");
	spdlog::set_pattern("[%H:%M:%e] [dap-rdebug] [%6t] [%L] %v");
	spdlog::set_level(static_cast<spdlog::level::level_enum>(config->getLogLevel()));

	//this->console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>("console");
	this->console = spdlog::stdout_color_mt("console");

	if (config->getLogFile() != "") {
		//this->fileLog = std::make_shared<spdlog::sinks::basic_file_sink_mt>("file", filename, true);
		this->fileLog = spdlog::basic_logger_mt("file", config->getLogFile(), true);

		if (config->getLogLevel() > 0) {
			this->fileLog->set_level(static_cast<spdlog::level::level_enum>(config->getLogLevel() - 1));
		}
		else {
			this->fileLog->set_level(spdlog::level::trace);
		}

		//delete this->allLoggers;
		//this->allLoggers = new spdlog::logger("all", { this->console, this->fileLog });
	}
	else {
	//this->allLoggers = new spdlog::logger("all", { this->console });
	}
}

Logger::~Logger() {
	if (!this->chronoMap.empty()) {
		std::string message = "There are unfinished chrono records. Labels: ";

		for (auto ci = this->chronoMap.begin(); ci != this->chronoMap.end(); ci++)
		{
			std::string label = ci->first;

			message.append(label);
			message.append(", ");
		}

		if (message.back() == ' ') {
			message.pop_back();
			message.pop_back();
		}

		this->chronoMap.clear();
		this->debug(message);
	}

	//this->console.reset();
	//this->console = nullptr;
	//this->fileLog.reset();
	//this->fileLog = nullptr;
}

void Logger::debug(std::string message, ...) {
	this->console->debug(message);

	if (this->fileLog != nullptr) {
		this->fileLog->debug(message);
	}

	//this->allLoggers->debug(message);
}

void Logger::info(std::string message, ...) {
	this->console->info(message);

	if (this->fileLog != nullptr) {
		this->fileLog->info(message);
	}

	//this->allLoggers->info(message);
}

void Logger::critical(std::string message, ...) {
	this->console->critical(message);

	if (this->fileLog != nullptr) {
		this->fileLog->critical(message);
	}

	//this->allLoggers->critical(message);
}

void Logger::error(std::string message, ...) {
	this->console->error(message);

	if (this->fileLog != nullptr) {
		this->fileLog->error(message);
	}

	//this->allLoggers->error(message);
}

void Logger::warn(std::string message, ...) {
	this->console->warn(message);

	if (this->fileLog != nullptr) {
		this->fileLog->warn(message);
	}

	//this->allLoggers->warn(message);
}

void Logger::startTrace(std::string label) {
	this->console->trace("[{}] start", label);

	if (this->fileLog != nullptr) {
		this->fileLog->trace("[{}] start", label);
	}

	this->startChrono(label);
	//this->allLoggers->trace(message);
}

void Logger::trace(std::string label, std::string message, ...) {
	this->console->trace(message);

	if (this->fileLog != nullptr) {
		this->fileLog->trace(message);
	}

	this->chrono(label);
	//this->allLoggers->trace(message);
}

void Logger::endTrace(std::string label) {
	this->console->trace("[{}] end", label);

	if (this->fileLog != nullptr) {
		this->fileLog->trace("[{}] end", label);
	}

	this->endChrono(label);
	//this->allLoggers->trace(message);
}

void Logger::startChrono(std::string label) {
	this->debug("chrono start: " + label);

	this->chronoMap.insert({ label, new spdlog::stopwatch() });
}

void Logger::chrono(std::string label) {
	if (this->chronoMap.contains(label)) {
		spdlog::stopwatch* sw = this->chronoMap.at(label);

		this->console->trace("[{0}] Partial {1:.3} ms", label.c_str(), sw->elapsed().count());
	}
}

void Logger::endChrono(std::string label) {
	if (this->chronoMap.contains(label)) {
		spdlog::stopwatch* sw = this->chronoMap.at(label);
		this->chronoMap.erase(label);

		this->console->trace("[{0}] Elpased {1:.3} ms", label.c_str(), sw->elapsed().count());

		delete sw;
	}
	else {
		this->console->trace("chrono label not found: {0}", label);
	}
}
