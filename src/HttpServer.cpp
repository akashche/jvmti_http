/* 
 * File:   HttpServer.cpp
 * Author: netbeans
 * 
 * Created on July 10, 2015, 7:23 PM
 */

#include <string>
#include <atomic>
#include <cstdlib>

#include "pion/tcp/connection.hpp"
#include "pion/http/request.hpp"
#include "pion/http/response_writer.hpp"

#include "staticlib/utils/tracemsg.hpp"
#include "staticlib/containers/blocking_queue.hpp"

#include "JvmtiHttpException.hpp"
#include "HttpServer.hpp"

namespace jvmti_http {

namespace ph = pion::http;

const uint32_t NUMBER_OF_ASIO_THREADS = 1;
const uint32_t QUEUE_MAX_SIZE = 1 << 20;

HttpServer::HttpServer(uint16_t port) :
server(1, port),
queue(1 << 10) {
    auto handler = [this](ph::request_ptr& req, pion::tcp::connection_ptr& conn) {
        // pion specific response writer creation
        auto finfun = std::bind(&pion::tcp::connection::finish, conn);
        auto writer = ph::response_writer::create(conn, *req, finfun);
        // reroute to worker
        this->queue.emplace(std::move(writer), req->get_resource().substr(1));
    };
    server.add_method_specific_resource("GET", "/", handler);
    try {
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
    // todo: investigate why blocks
//    queue.unblock();
}

void HttpServer::read_from_queue(jvmtiEnv* jvmti, JNIEnv* jni) {
    while (running.test_and_set()) {
        JvmtiHttpQuery el{};
        bool success = queue.take(el);
        if (success) {
            process_query(jvmti, jni, std::move(el));
        } else break;
    }
    running.clear();
}

void HttpServer::process_query(jvmtiEnv* jvmti, JNIEnv* jni, JvmtiHttpQuery&& query) {
    (void) jvmti;
    (void) jni;
    auto writer = query.get_writer();
    writer << "---\n";
    writer << "TODO: Implement me" << "\n";
    writer << "---\n";
    writer->send();
}

// jvmti access worker
//void JNICALL worker_fun(jvmtiEnv* jvmti, JNIEnv* /* jni */, void* /* p */) {
//    std::cout << "worker running" << std::endl;
//    for (;;) {
//        auto el = queue.take();
//        auto writer = el.get_writer();
//        char* buf = nullptr;
//        auto error = jvmti->GetSystemProperty(el.get_property().c_str(), &buf);
//        if (JVMTI_ERROR_NONE == error) {
//            std::string st{buf};
//            jvmti->Deallocate(reinterpret_cast<unsigned char*> (buf));
//            writer << "---\n";
//            writer << st << "\n";
//            writer << "---\n";
//        } else {
//            char* errbuf = nullptr;
//            jvmti->GetErrorName(error, &errbuf);
//            std::string st{errbuf};
//            writer << "***\n";
//            writer << st << "\n";
//            writer << "***\n";
//            jvmti->Deallocate(reinterpret_cast<unsigned char*> (errbuf));
//        }
//        // sending response to client
//        writer->send();
//    }
//}

} // namespace
