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
#include <cstdint>

#include <jvmti.h>

#include "staticlib/utils.hpp"

#include "HttpServer.hpp"
#include "JvmtiAccessor.hpp"

namespace { // anonymous

const uint16_t DEFAULT_PORT = 8080;

namespace su = staticlib::utils;
namespace sc = staticlib::containers;
namespace ph = pion::http;

// http server
jvmti_http::HttpServer* server;

// helper function
jthread alloc_thread(JNIEnv* env) {
    // todo: result checks
    jclass thrClass = env->FindClass("java/lang/Thread");
    jmethodID cid = env->GetMethodID(thrClass, "<init>", "()V");
    jthread res = env->NewObject(thrClass, cid);
    return res;
}

// required for worker init
static void JNICALL vm_init(jvmtiEnv *jvmti, JNIEnv *env, jthread /* thread */) {
    // todo: error checking
    jvmti->RunAgentThread(alloc_thread(env), jvmti_http::HttpServer::jvmti_callback, server, JVMTI_THREAD_NORM_PRIORITY);
}

jvmtiEnv* init_jvmti(JavaVM *jvm) {
    jvmtiEnv* jvmti;
    jvm->GetEnv((void **) &jvmti, JVMTI_VERSION);
    jvmtiCapabilities caps;
    memset(&caps, 0, sizeof (caps));
    // todo: refine me
    caps.can_generate_all_class_hook_events = 1;
    caps.can_tag_objects = 1;
    caps.can_get_source_file_name = 1;
    caps.can_get_line_numbers = 1;
    caps.can_generate_garbage_collection_events = 1;
    caps.can_tag_objects = 1;
    caps.can_generate_resource_exhaustion_heap_events = 1;
    auto error = jvmti->AddCapabilities(&caps);
    if (JVMTI_ERROR_NONE != error) {
        char* errbuf = nullptr;
        jvmti->GetErrorName(error, &errbuf);
        std::cout << "Capabilities error: " << errbuf << std::endl;
        jvmti->Deallocate(reinterpret_cast<unsigned char*> (errbuf));
    }
    return jvmti;
}

void add_jvmti_callback(jvmtiEnv* jvmti) {
    // worker callback
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof (callbacks));
    callbacks.VMInit = &vm_init;
    // todo: errors checking
    jvmti->SetEventCallbacks(&callbacks, sizeof (callbacks));
    jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, NULL);
}

uint16_t parse_port(char* options) {
    uint16_t port = DEFAULT_PORT;
    if (nullptr != options) {
        std::string optstr{options};
        if (optstr.size() > 0) {
            port = su::parse_uint16(optstr);
        }
    }
    return port;
}

} // namespace

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* jvm, char* options, void* /* reserved */) {
    jvmtiEnv * jvmti = init_jvmti(jvm);
    uint16_t port = parse_port(options);
    auto ja = new jvmti_http::JvmtiAccessor(jvmti);
    server = new jvmti_http::HttpServer{port, ja};
    add_jvmti_callback(jvmti);
    std::cout << "Agent HTTP server started on port: [" << port << "]" << std::endl;
    return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM* /* vm */) {
    server->shutdown();
    delete server;
    std::cout << "Agent HTTP server stopped" << std::endl;
}
