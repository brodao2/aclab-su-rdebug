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
#include <thread>
#include <io.h>
#include <fcntl.h>

#include "../include/dap/io.h"
#include "../include/dap/network.h"
#include "../include/dap/protocol.h"
#include "../include/dap/session.h"
#include "argparse.h"

#include "utils.h"
#include "logger.h"
#include "config.h"
#include <event.h>
#include <msg-initialize-request.h>
#include <msg-disconnect-request.h>
#include <msg-breakpoints-locations-request.h>
#include <rdebug-client.h>
#include <msg-attach-request.h>

#define VERSION "0.0.1"
#define APP_NAME "dap-rdebug"
#define APP_DESCRIPTION "AC Lab DAP-RDebug protocol tunnel (https://github.com/brodao2/aclab-su-rdebug)"
#define USAGE "usage: dap-rebug -h|--help" << std::endl

RDebugClient* client = nullptr;
enum class WaitServerEnum {
	WaitResponseServer,
	NormalProcess,
	DontWaitServer
};

enum class StoppedReasonEnum {
	Step,
	Breakpoint, 
	Exception, 
	Pause, 
	Entry,
	Goto,
	FunctionBreakpoint,
	DataBreakpoint,
	InstructionBreakpoint
};

WaitServerEnum waitServer = WaitServerEnum::DontWaitServer;

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
#ifndef DEBUG
		.default_value("trace");
#else
		.default_value("info");
#endif
	group.add_argument("--verbose")
		.help("increase output verbosity (equal --level=trace)")
		.default_value(false)
		.implicit_value(true);

	program.add_argument("--remote-port")
		.help("sketchUp remote port")
		.scan<'i', int>()
#ifndef DEBUG
		.default_value(9000);
#else
		.required();
#endif
	program.add_argument("-e", "--remote-workspace-root")
		.help("extension development path (root)")
#ifndef DEBUG
		.default_value("xxxxxxxxxxxxxx");
#else
		.required();
#endif

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		std::exit(1);
	}

	if (program.is_used("--remote-port")) {
		//a porta do DA é remote-port + 1
		int remotePort = program.get<int>("--remote-port");
		if ((remotePort < 1) || (remotePort > 65534)) //65535
		{
			std::cerr << APP_NAME << ": --remote-port out of range 1 and 65534" << std::endl;
			std::cerr << USAGE;

			return EXIT_FAILURE;
		}
	}

	/*
	* =============================================================================
	* Global Configuration
	* =============================================================================
	*/
	Config* config = Config::getInstance();

	config->setExtension(program.get<std::string>("--remote-workspace-root"));
	if (program.is_used("--verbose")) {
		config->setLogLevel(spdlog::level::trace);
	} else {
		config->setLogLevel(spdlog::level::from_str(program.get<std::string>("--level")));
	}
	config->setLogFile(program.get<std::string>("--log-to-file"));
	config->setRemotePort(program.get<int>("--remote-port"));

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
	for (size_t i = 0; i < argc; i++) {
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
	logger->info(std::string(". DA port       : ")
				 .append(std::to_string(config->getDebugAdapterPort())));
	logger->info(std::string(". Remote port   : ")
				 .append(std::to_string(config->getRemotePort())));
	logger->info(std::string(". Extension path: ")
				 .append(config->getExtension()));
	logger->info("-------------------------------------------------");

	logger->startTrace("main");

	const int kPort = config->getDebugAdapterPort();
	// Signal used to terminate the server session when a DisconnectRequest
	// is made by the client.
	std::condition_variable cvApp;
	std::mutex mutexApp;  // guards 'terminateApp'
	bool terminateApp = false;

	// Change stdin & stdout from text mode to binary mode.
	// This ensures sequences of \r\n are not changed to \n.
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);

	auto session = dap::Session::create();

	// Set the session to close on invalid data. This ensures that data received over the network
	// receives a baseline level of validation before being processed.
	session->setOnInvalidData(dap::kClose);

	// Signal used to terminate the server session when a DisconnectRequest
	// is made by the client.
	std::condition_variable cv;
	std::mutex mutex;  // guards 'terminate'
	bool terminate = false;

	//send Stopped event
	const auto sendStoppedEvent = ([&](StoppedReasonEnum reason, int breakpointIndex = -1) {
		dap::StoppedEvent event;
		switch (reason) {
			case StoppedReasonEnum::Step:
				event.reason = "step";
				break;
			case StoppedReasonEnum::Breakpoint:
				event.reason = "breakpoint";
				break;
			case StoppedReasonEnum::Exception:
				event.reason = "exception";
				break;
			case StoppedReasonEnum::Pause:
				event.reason = "pause";
				break;
			case StoppedReasonEnum::Entry:
				event.reason = "entry";
				break;
			case StoppedReasonEnum::Goto:
				event.reason = "goto";
				break;
			case StoppedReasonEnum::FunctionBreakpoint:
				event.reason = "function breakpoint";
				break;
			case StoppedReasonEnum::DataBreakpoint:
				event.reason = "data breakpoint";
				break;
			case StoppedReasonEnum::InstructionBreakpoint:
				event.reason = "instruction breakpoint";
				break;
			default:
				break;
		}
		event.threadId = 1;
		if (breakpointIndex != -1) {
			event.hitBreakpointIds = { breakpointIndex };
		}
		session->send(event);
	});

	// The Initialize request is the first message sent from the client and
	// the response reports debugger capabilities.
	// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Initialize
	session->registerHandler([&](const dap::InitializeRequest& message) {
		logger->startTrace("initializeRequest");
		dap::InitializeResponse response = MessageInitializeRequest::Run(message);

		if (config->isWaitAttach()) {
			logger->debug("waitting attach...");
			int x = 20;
			while (x > 0) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				x--;
			}
		}

		client = RDebugClient::createRDebugClient(config->getRemotePort());
		if (client == nullptr) {
			logger->error("Connection to RDebug port failure.");
			exit(10);
		}

		dap::InitializedEvent initializedEvent;
		session->send(initializedEvent);

		logger->endTrace("initializeRequest");

		return response;
	});

	session->registerHandler([&](const dap::TerminateRequest& message) {
		logger->startTrace("terminateRequest");
		dap::TerminateResponse response;

		std::unique_lock<std::mutex> lock(mutex);
		terminate = true;
		cv.notify_one();

		logger->endTrace("terminateRequest");
		return response;
	});

	session->registerHandler([&](const dap::AttachRequest& message) {
		logger->startTrace("attachRequest");
		dap::AttachResponse response = MessageAttachRequest::Run(message);

		client->start();

		logger->endTrace("attachRequest");
		return response;
	});

	session->registerHandler([&](const dap::LaunchRequest& message) {
		logger->startTrace("launchRequest");
		dap::LaunchResponse response;

		client->start();

		logger->endTrace("launchRequest");
		return response;
	});

	//session->registerHandler([&](const dap::BreakpointLocationsRequest& message) {
	//	logger->startTrace("breakpointLocationsRequest");
	//	dap::BreakpointLocationsResponse response = MessageBreakpointLocationsRequest::Run(message);
	//	
	//	if (client->addBreakpoint(message.source.path.value(), message.line, "")) {
	//		//client->continue_();
	//	}
	//	
	//	logger->endTrace("breakpointLocationsRequest");

	//	return response;
	//	});

	session->registerHandler([&](const dap::SetBreakpointsRequest& message) {
		logger->startTrace("setBreakpointsRequest");
		dap::SetBreakpointsResponse response;
		dap::array<dap::SourceBreakpoint> breakpoints = message.breakpoints.value();

		if (client->removeBreakpoint(message.source.path.value())) {
			for (auto sourceBreakpoint_i = breakpoints.begin(); sourceBreakpoint_i != breakpoints.end(); sourceBreakpoint_i++) {
				dap::SourceBreakpoint breakpoint = *sourceBreakpoint_i;
				dap::Breakpoint addedBreakpoint;

				addedBreakpoint.line = breakpoint.line;
				addedBreakpoint.verified = client->addBreakpoint(message.source.path.value(), breakpoint.line,
																 breakpoint.condition.has_value() ? breakpoint.condition.value() : "");
				if (!addedBreakpoint.verified) {
					addedBreakpoint.message = "Error addBreakpoint";
				}

				response.breakpoints.emplace_back(addedBreakpoint);
			}
		} else {
			logger->error("Não foi possível remover BP de ?????");
		}


		logger->endTrace("SetBreakpointsRequest");

		return response;
	});

	session->registerHandler([&](const dap::ConfigurationDoneRequest& message) {
		logger->startTrace("configurationDoneRequest");
		dap::ConfigurationDoneResponse response;

		logger->endTrace("configurationDoneRequest");

		return response;
	});

	session->registerHandler([&](const dap::ThreadsRequest& message) {
		logger->startTrace("threadsRequest");
		dap::ThreadsResponse response;
		dap::array<dap::Thread> threads;
		std::vector<Thread> threadList;

		if (client->threads(threadList)) {
			for (auto threads_i = threadList.begin(); threads_i != threadList.end(); threads_i++) {
				Thread thread = *threads_i;
				dap::Thread threadResponse;

				threadResponse.id = thread.id;
				threadResponse.name = thread.name;

				threads.emplace_back(threadResponse);
			}
		}

		response.threads = threads;

		logger->endTrace("threadsnDoneRequest");

		return response;							 }
	);

	session->registerHandler([&](const dap::StackTraceRequest& message) {
		dap::StackTraceResponse response;
		logger->startTrace("stackTraceRequest");

		dap::array<dap::StackFrame> frames;
		std::vector<Frame> frameList;

		if (client->where(frameList)) {
			for (auto frames_i = frameList.begin(); frames_i != frameList.end(); frames_i++) {
				Frame frame = *frames_i;

				dap::Source source;
				source.path = frame.source;

				dap::StackFrame frameResponse;
				frameResponse.id = frame.id;
				int pos = frame.source.find_last_of("/");
				pos = frame.source.find_last_of(frame.source.size() - pos);
				frameResponse.name = frame.source.substr(pos);
				frameResponse.source = source;
				frameResponse.line = frame.line;

				frames.emplace_back(frameResponse);
			}
		}

		response.stackFrames = frames;
		response.totalFrames = frames.size();

		logger->endTrace("stackTraceRequest");

		return response;							 }
	);

	session->registerHandler([&](const dap::ScopesRequest& message) {
		dap::ScopesResponse response;
		logger->startTrace("scopesRequest");

		//message.frameId
		dap::array<dap::Scope> scopes;
		std::vector<Scope*> scopeList;
		dap::Source source;
		source.path = "";

		if (client->scopes(scopeList)) {
			for (auto scopes_i = scopeList.begin(); scopes_i != scopeList.end(); scopes_i++) {
				Scope* scope = *scopes_i;
				dap::Scope scopeResponse;

				scopeResponse.name = scope->name;
				scopeResponse.expensive = true;
				scopeResponse.namedVariables = scope->namedVariables;
				scopeResponse.indexedVariables = scope->indexedVariables;
				//scopeResponse.source = source;
				scopeResponse.variablesReference = scope->referenceId;

				scopes.emplace_back(scopeResponse);
			}
		}

		response.scopes = scopes;

		logger->endTrace("scopesRequest");

		return response;							 }
	);


	session->registerHandler([&](const dap::VariablesRequest& message) {
		dap::VariablesResponse response;
		logger->startTrace("variablesRequest");
		std::vector<dap::Variable> variables;

		Scope* scope = client->findScopeMap(message.variablesReference);
		if (scope != nullptr) {
			if (client->getVariables(scope)) {
				for (auto variable_i = scope->variables.begin(); variable_i != scope->variables.end(); variable_i++) {
					Variable* variable = *variable_i;
					dap::Variable variableResponse;

					variableResponse.name = variable->name;
					variableResponse.evaluateName = variable->evaluateName;
					variableResponse.type = variable->type;
					//variableResponse.expensive = true;
					variableResponse.namedVariables = variable->namedVariables;
					variableResponse.indexedVariables = variable->indexedVariables;
					//scopeResponse.source = source;
					variableResponse.variablesReference = variable->referenceId;
					//scopeResponse.presentationHint = "local";
					variableResponse.value = variable->value;

					variables.emplace_back(variableResponse);
				}
			}
		}

		response.variables = variables;

		logger->endTrace("variablesRequest");

		return response;							 }
	);

	session->registerHandler([&](const dap::NextRequest& message) {
		dap::NextResponse response;
		logger->startTrace("nextRequest");

		if (client->next()) {
			sendStoppedEvent(StoppedReasonEnum::Step);
		}

		logger->endTrace("nextRequest");

		return response;							 }
	);
	
	session->registerHandler([&](const dap::StepOutRequest& message) {
		dap::StepOutResponse response;
		logger->startTrace("stepOutRequest");

		if (client->stepOut()) {
			sendStoppedEvent(StoppedReasonEnum::Step);
		}

		logger->endTrace("stepOutRequest");

		return response;							 }
	);

	session->registerHandler([&](const dap::PauseRequest& message) {
		dap::PauseResponse response;
		logger->startTrace("pauseRequest");

		if (client->interrupt()) {
			sendStoppedEvent(StoppedReasonEnum::Pause);
		}

		logger->endTrace("pauseRequest");

		return response;							 }
	);

	session->registerHandler([&](const dap::ContinueRequest& message) {
		dap::ContinueResponse response;
		logger->startTrace("continueRequest");

		if (client->continue_()) {
			waitServer == WaitServerEnum::DontWaitServer;
		}

		logger->endTrace("continueRequest");

		return response;							 }
	);


	session->registerSentHandler([&](const dap::ResponseOrError<dap::ThreadsResponse>& message) {
		if (waitServer == WaitServerEnum::DontWaitServer) {
			waitServer = WaitServerEnum::WaitResponseServer;
		}
	});

	// The Disconnect request is made by the client before it disconnects
	// from the server.
	// https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Disconnect
	session->registerHandler([&](const dap::DisconnectRequest& message) {
		logger->startTrace("disconnectRequest");
		// Client wants to disconnect. Set terminate to true, and signal the
		// condition variable to unblock the server thread.
		dap::DisconnectResponse response = MessageDisconnectRequest::Run(message);

		if (client) {
			client->disconnect();

			delete client;
			client = nullptr;
		}

		logger->endTrace("disconnectRequest");
		return response;
	});

	// Error handler
	auto onError = [&](const char* msg) {
		logger->error("Server error:");
		logger->error(msg);
	};

	// Create the network server
	std::shared_ptr<dap::Reader> in = dap::file(stdin, true);
	std::shared_ptr<dap::Writer> out = dap::file(stdout, true);
	session->bind(in, out);

	// Wait for the client to disconnect (or reach a 5 second timeout)
	// before releasing the session and disconnecting the socket to the
	// client.
	std::unique_lock<std::mutex> lock(mutex);
	while (!terminate) {
		cv.wait_for(lock, std::chrono::seconds(3), [&] { return terminate; });
		std::cerr << "App Waiting message " << terminate << std::endl;

		if (waitServer == WaitServerEnum::WaitResponseServer) {
			Breakpoint* breakpoint = client->waitBreakpoint();

			if (breakpoint) {
				waitServer = WaitServerEnum::NormalProcess;

				dap::BreakpointEvent event;
				dap::Source source;
				source.path = breakpoint->source;

				event.breakpoint.source = source;
				event.breakpoint.line  = breakpoint->line;

				session->send(event);
				sendStoppedEvent(StoppedReasonEnum::Breakpoint, 1);
			}
		}
	};

	auto waitProcess = [&]() {
		std::unique_lock<std::mutex> lockApp(mutexApp);

		while (!terminateApp) {
			cvApp.wait_for(lockApp, std::chrono::seconds(5), [&] { return terminateApp; });
			std::cerr << "App Waiting message " << terminateApp << std::endl;
		}
	};

	std::thread t1(waitProcess);
	t1.join();

	logger->info("Server stopping...");
	logger->endTrace("main");

	logger->info("Shutdown");

	return 0;
}

