/* 
 * File:   main.cpp
 * Author: netbeans
 *
 * Created on July 1, 2015, 1:44 PM
 */

#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

#include "staticlib/utils.hpp"
#include "staticlib/containers.hpp"

#include "pion/http/streaming_server.hpp"
#include "pion/tcp/connection.hpp"
#include "pion/http/request.hpp"
#include "pion/http/response_writer.hpp"


namespace { // anonymous

const uint16_t TCP_PORT = 8080;
namespace su = staticlib::utils;
namespace sc = staticlib::containers;

} // namespace

int main() {
    sc::blocking_queue<pion::http::response_writer_ptr> queue{1 << 10};
    // start worker thread
    std::thread worker([&] {
        for (;;) {
            auto writer = queue.take();
            writer << "---\n";
            writer << "Hi from worker" << "\n";
            writer << "---\n";
            // sending response to client
            writer->send();
        }
    });
    worker.detach();
    
    // start server
    PION_LOG_SETLEVEL_WARN(PION_GET_LOGGER("pion"));
    pion::http::streaming_server server{1, TCP_PORT};
    server.add_resource("/jvmti", [&](pion::http::request_ptr& req, pion::tcp::connection_ptr& conn) {
        // pion specific response writer creation
        auto finfun = std::bind(&pion::tcp::connection::finish, conn);
        auto writer = pion::http::response_writer::create(conn, *req, finfun);
        // reroute to worker
        queue.emplace(std::move(writer));
    });
    server.start();
    // wait for Ctrl+c
    su::initialize_signals();
    su::wait_for_signal();
    // stop server
    server.stop();
    return 0;
}

