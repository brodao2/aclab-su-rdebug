// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// simple_net_client_server demonstrates a minimal DAP network connection
// between a server and client.

#include <condition_variable>
#include <mutex>
#include <iostream>

#include "../include/dap/io.h"
#include "../include/dap/network.h"
#include "../include/dap/protocol.h"
#include "../include/dap/session.h"
#include "third_party/cxxopts/include/cxxopts.hpp"

#include "utils.h"
#include "logger.h"
#include "config.h"

#define VERSION "0.0.1"
#define APP_NAME "dap-rdebug"
#define APP_DESCRIPTION "AC Lab DAP-RDebug protocol tunnel (https://github.com/brodao2/aclab-su-rdebug)"
#define USAGE "usage: dap-rebug --help" << std::endl

int main(int argc, char* argv[], char* envp[]) {
	cxxopts::Options options(APP_NAME,
		std::string(APP_DESCRIPTION).append(" v. ").append(VERSION)
	);

	options.add_options()
		("v,version", "Show the version")
		("h,help", "Show the help")
		("l,level", "Log level (critical|error|warn|info|debug|trace|OFF)", cxxopts::value<std::string>())
		("log-to-file", "Filename to record log of the DA executable file.", cxxopts::value<std::string>());

	options.add_options("Required")
		("remote-port", "SketchUp Remote port", cxxopts::value<int>())
		("extension-development-path", "Extension Development Path (root)", cxxopts::value<std::string>());

	options.add_options("Sketchup")
		("sketchup-program", "Absolute path to a SketchUp executable file", cxxopts::value<std::string>())
		("sketchup-arguments", "SketchUp program arguments (comma separated)", cxxopts::value<std::vector<std::string>>());

	//options.parse_positional({ "input_files" });

	cxxopts::ParseResult result = options.parse(argc, argv);
	//options.allow_unrecognised_options();

	try {
		result = options.parse(argc, argv);
	}
	catch (const cxxopts::exceptions::specification& x) {
		std::cerr << APP_NAME << ": " << x.what() << '\n';
		std::cerr << USAGE;
		return EXIT_FAILURE;
	}

	//auto remaining = result.unmatched();
	//for (auto& el : remaining) {
	//    std::cout << el << " ";
	//}

	const std::vector<std::string> unknowOptios = result.unmatched();
	if (unknowOptios.size() > 0) {
		std::cerr << APP_NAME << ": unknow options "
			<< join(unknowOptios.begin(), unknowOptios.end(), ", ")
			<< std::endl;
		std::cerr << USAGE;

		return EXIT_FAILURE;
	}

	if (result.count("help"))
	{
		std::cout << options.help() << std::endl;
		return EXIT_SUCCESS;
	}

	if (result.count("version"))
	{
		std::cout << APP_NAME << " << " VERSION << std::endl;
		return EXIT_SUCCESS;
	}

	if (result.count("remote-port") != 0)
	{
		//a porta do DA é port-remote + 1

		if ((result["remote-port"].as<int>() < 1024) || (result["remote-port"].as<int>() > 65534)) //65535
		{
			std::cerr << APP_NAME << ": remote-port option out of range 10124 and 65534" << std::endl;
			std::cerr << USAGE;

			return EXIT_FAILURE;
		}
	}

	if (result.count("extension-development-path") == 0)
	{
		std::cerr << APP_NAME << ": extension-development-path option is required" << std::endl;
		std::cerr << USAGE;

		return EXIT_FAILURE;
	}

	if ((result.count("sketchup-arguments") != 0) && (result.count("sketchup-program") == 0))
	{
		std::cerr << APP_NAME << ": sketchup-program option is required for sketchup-arguments" << std::endl;
		std::cerr << USAGE;

		return EXIT_FAILURE;
	}

	Config* config = Config::getInstance();

	config->setExtension(result["extension-development-path"].as<std::string>());

	if (result.count("level")) {
		config->setLogLevel(spdlog::level::from_str(result["level"].as<std::string>()));
	}
	if (result.count("log-to-file")) {
		config->setLogFile(result["log-to-file"].as<std::string>());
	}
	if (result.count("remote-port")) {
		config->setRemotePort(result["remote-port"].as<int>());
	}
	if (result.count("sketchup-program")) {
		config->setSketchupProgram(result["sketchup-program"].as<std::string>());
	}
	if (result.count("sketchup-arguments")) {
		config->setSketchupArguments(result["sketchup-arguments"].as<std::vector<std::string>>());
	}

	Logger* logger = Logger::getInstance();

	logger->info(
		std::string("Program: ")
		.append(APP_NAME)
		.append(" ")
		.append(APP_DESCRIPTION)
		.append(" v. ")
		.append(VERSION)
	);

	logger->info(
		std::string("Arguments: ")
		.append(std::to_string(argc))
	);
	for (size_t i = 0; i < argc; i++)
	{
		logger->info(
			std::string(" . [")
			.append(std::to_string(i))
			.append("] : ")
			.append(std::string(argv[i]))
		);
	}

	logger->info("Config: ");
	logger->info(std::string(". Level log     : ")
		.append(spdlog::level::to_string_view(config->getLogLevel()).data())
	);
	logger->info(std::string(". Filename log  : ")
		.append("(")
		.append(config->getLogLevel() > 0 ? 
			spdlog::level::to_string_view(static_cast<spdlog::level::level_enum>(config->getLogLevel()-1)).data()
			: spdlog::level::to_string_view(static_cast<spdlog::level::level_enum>(config->getLogLevel())).data()
		)
		.append(") ")
		.append(config->getLogFile())
	);
	logger->info(std::string(". Remote port   : ")
		.append(std::to_string(config->getRemotePort())));
	logger->info(std::string(". Extension path: ")
		.append(config->getExtension()));
	logger->info(std::string(". Sketchup path : ")
		.append(config->getSketchupProgram()));
	std::vector<std::string> sketchupArguments = config->getSketchupArguments();
	logger->info(std::string(". Sketchup arguments: ")
		.append(std::to_string(sketchupArguments.size())));
	for (size_t i = 0; i < sketchupArguments.size(); i++)
	{
		logger->info(
			std::string("  . [")
			.append(std::to_string(i))
			.append("] : ")
			.append(std::string(sketchupArguments[i]))
		);
	}
	logger->info("-------------------------------------------------");

	logger->startTrace("main");

	const int kPort = config->getRemotePort() + 1;

	// Callback handler for a socket connection to the server
	auto onClientConnected =
		[&](const std::shared_ptr<dap::ReaderWriter>& socket) {
		auto session = dap::Session::create();

		// Set the session to close on invalid data. This ensures that data received over the network
		// receives a baseline level of validation before being processed.
		session->setOnInvalidData(dap::kClose);

		session->bind(socket);

		// The Initialize request is the first message sent from the client and
		// the response reports debugger capabilities.
		// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Initialize
		session->registerHandler([&](const dap::InitializeRequest&) {
			dap::InitializeResponse response;
			logger->info("Server received initialize request from client");

			return response;
			});

		// Signal used to terminate the server session when a DisconnectRequest
		// is made by the client.
		bool terminate = false;
		std::condition_variable cv;
		std::mutex mutex;  // guards 'terminate'

		// The Disconnect request is made by the client before it disconnects
		// from the server.
		// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Disconnect
		session->registerHandler([&](const dap::DisconnectRequest&) {
			// Client wants to disconnect. Set terminate to true, and signal the
			// condition variable to unblock the server thread.
			std::unique_lock<std::mutex> lock(mutex);
			terminate = true;
			cv.notify_one();
			return dap::DisconnectResponse{};
			});

		// Wait for the client to disconnect (or reach a 5 second timeout)
		// before releasing the session and disconnecting the socket to the
		// client.
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait_for(lock, std::chrono::seconds(5), [&] { return terminate; });
		logger->info("Server closing connection");
		logger->trace("main", "close");
		};

	// Error handler
	auto onError = [&](const char* msg) {
		logger->error("Server error: %s", msg);
		};

	// Create the network server
	auto server = dap::net::Server::create();
	// Start listening on kPort.
	// onClientConnected will be called when a client wants to connect.
	// onError will be called on any connection errors.
	server->start(kPort, onClientConnected, onError);

	// Create a socket to the server. This will be used for the client side of the
	// connection.
	auto client = dap::net::connect("localhost", kPort);
	if (!client) {
		logger->error("Couldn't connect to server");
		return 1;
	}

	// Attach a session to the client socket.
	auto session = dap::Session::create();
	session->bind(client);

	// Set an initialize request to the server.
	auto future = session->send(dap::InitializeRequest{});
	logger->info("Client sent initialize request to server");
	logger->info("Waiting for response from server...");
	// Wait on the response.
	auto response = future.get();
	logger->trace("main", "Response received from server");
	logger->info("Disconnecting...");
	// Disconnect.
	session->send(dap::DisconnectRequest{});

	logger->endTrace("main");

	return 0;
}
