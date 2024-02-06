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
#include "argparse.h"

#include "utils.h"
#include "logger.h"
#include "config.h"

#define VERSION "0.0.1"
#define APP_NAME "dap-rdebug"
#define APP_DESCRIPTION "AC Lab DAP-RDebug protocol tunnel (https://github.com/brodao2/aclab-su-rdebug)"
#define USAGE "usage: dap-rebug -h|--help" << std::endl

int main(int argc, char* argv[], char* envp[]) {
	/*
	* =============================================================================
	* Command Line Interface (CLI)
	* =============================================================================
	*/
	argparse::ArgumentParser program(APP_NAME, VERSION);

	program.add_description(APP_DESCRIPTION);
	program.add_epilog("Thanks for using " + std::string(APP_NAME) + ". Contributions are welcome.");

	program.add_argument("--log-to-file")
		.help("filename to record log of the DA executable file")
		.default_value("");

	argparse::ArgumentParser::MutuallyExclusiveGroup& group = program.add_mutually_exclusive_group();
	group.add_argument("-l", "--level")
		.help("log level (critical|error|warn|info|debug|trace|OFF)")
		.default_value("info");
	group.add_argument("--verbose")
		.help("increase output verbosity (equal --level=trace)")
		.default_value(false)
		.implicit_value(true);

	program.add_argument("--remote-port")
		.help("sketchUp remote port")
		.scan<'i', int>()
		.required();
	program.add_argument("-e", "--extension-development-path")
		.help("extension development path (root)")
		.required();

	program.add_argument("--su", "--sketchup-program")
		.help("absolute path to a SketchUp executable file")
		.default_value("");
	program.add_argument("--su-arg", "--sketchup-argument")
		.help("sketchUp program arguments (one for each)")
		.append()
		.default_value<std::vector<std::string>>({ })
		.nargs(0, 5);

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		std::exit(1);
	}

	if (program.present<int>("--remote-port"))
	{
		//a porta do DA é remote-port + 1
		int remotePort = program.get<int>("--remote-port");
#ifdef LINUX
		if ((remotePort < 1024) || (remotePort > 65534)) //65535
		{
			std::cerr << APP_NAME << ": --remote-port out of range 10124 and 65534" << std::endl;
			std::cerr << USAGE;

			return EXIT_FAILURE;
		}
#else
		if ((remotePort < 1) || (remotePort > 65534)) //65535
		{
			std::cerr << APP_NAME << ": --remote-port out of range 1 and 65534" << std::endl;
			std::cerr << USAGE;

			return EXIT_FAILURE;
		}
#endif
	}

	//auto suArgs = program.get<std::vector<std::string>>("--sketchup-arguments");
	//if ((suArgs.size() > 0) && (!program.present("--sketchup-program")))
	//{
	//	std::cerr << APP_NAME << ": --sketchup-program is required for --sketchup-arguments" << std::endl;
	//	std::cerr << USAGE;

	//	return EXIT_FAILURE;
	//}

	/*
	* =============================================================================
	* Global Configuration
	* =============================================================================
	*/
	Config* config = Config::getInstance();

	config->setExtension(program.get<std::string>("--extension-development-path"));
	if (program.is_used("--verbose")) {
		config->setLogLevel(spdlog::level::trace);
	} else {
		config->setLogLevel(spdlog::level::from_str(program.get<std::string>("--level")));
	}
	config->setLogFile(program.get<std::string>("--log-to-file"));
	config->setRemotePort(program.get<int>("--remote-port"));
	config->setSketchupProgram(program.get<std::string>("--sketchup-program"));
	//config->setSketchupArguments(program.get<std::vector<std::string>>("--sketchup-arguments"));

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
			spdlog::level::to_string_view(static_cast<spdlog::level::level_enum>(config->getLogLevel() - 1)).data()
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

	const int kPort = config->getRemotePort();

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
