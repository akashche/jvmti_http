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
#include <cstdlib>

#include <jvmti.h>

#include "pion/http/streaming_server.hpp"

#include "staticlib/containers/blocking_queue.hpp"

#include "JvmtiHttpQuery.hpp"


namespace jvmti_http {

class HttpServer {
    pion::http::streaming_server server;
    staticlib::containers::blocking_queue<JvmtiHttpQuery> queue;
    std::atomic_flag running = ATOMIC_FLAG_INIT;
    
public:
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    
    HttpServer(uint16_t port);
    
    static void JNICALL jvmti_callback(jvmtiEnv* jvmti, JNIEnv* jni, void* user_data);
    
    void shutdown();
    
private:
    
    void read_from_queue(jvmtiEnv* jvmti, JNIEnv* jni);
    
    void process_query(jvmtiEnv* jvmti, JNIEnv* jni, JvmtiHttpQuery&& query);
    
};

}

#endif	/* JVMTI_HTTP_HTTPSERVER_HPP */

