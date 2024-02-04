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

#include "../include/dap/io.h"
#include "../include/dap/network.h"
#include "../include/dap/protocol.h"
#include "../include/dap/session.h"

#include <condition_variable>
#include <mutex>

#include "logger.h"
int main(int, char* []) {
    Logger* logger = Logger::getInstance();
    logger->_startTrace("main");

    logger->info("antes de setar arquivo");
    logger->info("=====================");
    logger->info("=====================");
    logger->info("=====================");
    logger->info("=====================");
    logger->info("=====================");

    logger->setFilename("./dap-rdebug.log");
    logger->info("Começando ....");

    logger->debug    ("SPDLOG_LEVEL_DEBUG");
    logger->info     ("SPDLOG_LEVEL_INFO");
    logger->warn     ("SPDLOG_LEVEL_WARN");
    logger->error      ("SPDLOG_LEVEL_ERROR");
    logger->critical ("SPDLOG_LEVEL_CRITICAL");
    //logger->off = SPDLOG_LEVEL_OFF,
    logger->_trace("main", "SPDLOG_LEVEL_TRACE");

    constexpr int kPort = 19021;

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
        logger->_trace("main", "close");
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
    logger->_trace("main", "Response received from server");
    logger->info("Disconnecting...");
    // Disconnect.
    session->send(dap::DisconnectRequest{});

    logger->_endTrace("main");

    return 0;
}
