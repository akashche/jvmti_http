/*
 * Copyright 2015, akashche at redhat.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   HttpServer.cpp
 * Author: akashche
 * 
 * Created on July 10, 2015, 7:23 PM
 */

#include <cstdlib>
#include <string>
#include <atomic>
#include <functional>
#include <fstream>
#include <memory>

#include "staticlib/config.hpp"
#include "staticlib/httpserver.hpp"
#include "staticlib/containers/blocking_queue.hpp"

#include "JvmtiHttpException.hpp"
#include "HttpServer.hpp"

namespace jvmti_http {

namespace sh = staticlib::httpserver;

const uint32_t NUMBER_OF_ASIO_THREADS = 1;
const uint32_t QUEUE_MAX_SIZE = 1 << 20;
const std::string HANDLERS_URL = "/jvmti/";
const std::string WEBAPP_URL = "/webapp/";

HttpServer::HttpServer(uint16_t port, JvmtiAccessor* ja, const std::string& webapp_zip_path,
        const std::string& cert_path) :
ja(ja),
queue(1 << 10),
server(2, port, asio::ip::address_v4::any(), cert_path),
webapp_resource(webapp_zip_path, WEBAPP_URL) {
#ifdef STATICLIB_WINDOWS
    // http://stackoverflow.com/a/19658185/314015
    running.clear();
#endif    
    auto handler = [this](sh::http_request_ptr& req, sh::tcp_connection_ptr& conn) {
        auto writer = sh::http_response_writer::create(conn, req);
        // reroute to worker
        this->queue.emplace(std::move(writer), req->get_resource().substr(HANDLERS_URL.length()));
    };
    auto webapp_handler = [this](sh::http_request_ptr& req, sh::tcp_connection_ptr& conn) {
        this->webapp_resource.handle(req, conn);
    };
    try {
        server.add_handler("GET", HANDLERS_URL, handler);
        server.add_handler("GET", WEBAPP_URL, webapp_handler);
        server.start();
        running.test_and_set();
    } catch (const std::exception& e) {
        throw JvmtiHttpException(TRACEMSG(e.what()));
    }
}

void JNICALL HttpServer::jvmti_callback(jvmtiEnv* jvmti, JNIEnv* jni, void* user_data) {
    auto hs_ptr = reinterpret_cast<HttpServer*>(user_data);
    hs_ptr->read_from_queue(jvmti, jni);

}

void HttpServer::shutdown() {
    server.stop(false);
    running.clear();
    // todo: investigate why blocks on linux
#ifdef STATICLIB_WINDOWS
    queue.unblock();
#endif
}

void HttpServer::read_from_queue(jvmtiEnv* jvmti, JNIEnv* jni) {
    while (running.test_and_set()) {
        detail::Query el{};
        bool success = queue.take(el);
        if (success) {
            try {
                auto resp = ja->process_query(jvmti, jni, el.get_property());
                el.get_writer()->write_move(std::move(resp));
            } catch (const std::exception& e) {
                el.get_writer() << TRACEMSG(e.what()) << "\n";
            }
            el.get_writer()->send();
        } else break;
    }
    running.clear();
}

} // namespace
