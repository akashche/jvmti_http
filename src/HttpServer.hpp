/* 
 * File:   HttpServer.hpp
 * Author: netbeans
 *
 * Created on July 10, 2015, 7:23 PM
 */

#ifndef JVMTI_HTTP_HTTPSERVER_HPP
#define	JVMTI_HTTP_HTTPSERVER_HPP

#include <string>
#include <atomic>
#include <memory>
#include <cstdlib>

#include <jvmti.h>

#include "pion/http/streaming_server.hpp"
#include "pion/http/response_writer.hpp"

#include "staticlib/containers/blocking_queue.hpp"

#include "JvmtiAccessor.hpp"

namespace jvmti_http {

namespace detail {

class Query {
    pion::http::response_writer_ptr writer;
    std::string property;

public:
    Query(const Query&) = delete;
    Query& operator=(const Query&) = delete;

    Query(Query&& other) :
    writer(std::move(other.writer)),
    property(std::move(other.property)) { }

    Query& operator=(Query&& other) {
        this->writer = std::move(other.writer);
        this->property = std::move(other.property);
        return *this;
    }

    Query() { }

    Query(pion::http::response_writer_ptr writer, std::string property) :
    writer(std::move(writer)),
    property(std::move(property)) { }

    pion::http::response_writer_ptr& get_writer() {
        return writer;
    }

    const std::string& get_property() {
        return property;
    }
};

}

class HttpServer {
    pion::http::streaming_server server;
    staticlib::containers::blocking_queue<detail::Query> queue;
    std::atomic_flag running = ATOMIC_FLAG_INIT;
    std::unique_ptr<JvmtiAccessor> ja;
    
public:
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    
    HttpServer(uint16_t port, JvmtiAccessor* ja);
    
    static void JNICALL jvmti_callback(jvmtiEnv* jvmti, JNIEnv* jni, void* user_data);
    
    void shutdown();
    
private:
    
    void read_from_queue(jvmtiEnv* jvmti, JNIEnv* jni);
};

}

#endif	/* JVMTI_HTTP_HTTPSERVER_HPP */

