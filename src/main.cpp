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
#include <atomic>

#include "staticlib/utils.hpp"
#include "staticlib/utils/ProducerConsumerQueue.hpp"

#include "pion/http/streaming_server.hpp"
#include "pion/tcp/connection.hpp"
#include "pion/http/request.hpp"
#include "pion/http/response_writer.hpp"


namespace { // anonymous

const uint16_t TCP_PORT = 8080;
namespace su = staticlib::utils;


} // namespace

int main() {
    // init data and queue
    std::atomic_flag running = ATOMIC_FLAG_INIT;
    running.test_and_set();
    su::ProducerConsumerQueue<pion::http::response_writer_ptr> queue{10};
    // start worker thread
    std::thread worker{[&] {
        pion::http::response_writer_ptr writer{};
        while(running.test_and_set()) {
            bool success = queue.read(writer);
            if (success) {
                writer << "---\n";
                writer << "Hi from worker" << "\n";
                writer << "---\n";
                // sending response to client
                writer->send();
            }
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
        std::cout << "worker exited" << std::endl;
    }};
    
    // start server
    PION_LOG_SETLEVEL_WARN(PION_GET_LOGGER("pion"));
    pion::http::streaming_server server{1, TCP_PORT};
    server.add_resource("/jvmti", [&](pion::http::request_ptr& req, pion::tcp::connection_ptr& conn) {
        // pion specific response writer creation
        auto finfun = std::bind(&pion::tcp::connection::finish, conn);
        auto writer = pion::http::response_writer::create(conn, *req, finfun);
        // reroute to worker
        queue.write(writer);
    });
    server.start();
    // wait for Ctrl+c
    su::initialize_signals();
    su::wait_for_signal();
    // stop server
    running.clear();
    worker.join();
    server.stop(true);
    return 0;
}

