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

#include <jvmti.h>

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
namespace ph = pion::http;

// forward declaration
class JvmtiHttpQuery;

// http server
ph::streaming_server* server;
// writers queue
sc::blocking_queue<JvmtiHttpQuery> queue{1 << 10};

// JVMTI globals
jvmtiEnv* jvmti = nullptr;
jvmtiCapabilities caps;
jvmtiEventCallbacks callbacks;

// queue elements type definition
class JvmtiHttpQuery {
    ph::response_writer_ptr writer;
    std::string property;

public:
    JvmtiHttpQuery(const JvmtiHttpQuery&) = delete;
    JvmtiHttpQuery& operator=(const JvmtiHttpQuery&) = delete;

    JvmtiHttpQuery(JvmtiHttpQuery&& other) :
    writer(std::move(other.writer)),
    property(std::move(other.property)){}

    JvmtiHttpQuery& operator=(JvmtiHttpQuery&& other) {
        this->writer = std::move(other.writer);
        this->property = std::move(other.property);
        return *this;
    }

    explicit JvmtiHttpQuery(ph::response_writer_ptr writer, std::string property) :
    writer(std::move(writer)),
    property(std::move(property)) { }

    ph::response_writer_ptr& get_writer() {
        return writer;
    }

    std::string& get_property() {
        return property;
    }
};

// helper function
jthread alloc_thread(JNIEnv* env) {
    // todo: result checks
    jclass thrClass = env->FindClass("java/lang/Thread");
    jmethodID cid = env->GetMethodID(thrClass, "<init>", "()V");
    jthread res = env->NewObject(thrClass, cid);
    return res;
}

// jvmti access worker
void JNICALL worker_fun(jvmtiEnv* jvmti, JNIEnv* /* jni */, void* /* p */) {
    std::cout << "worker running" << std::endl;
    for (;;) {
        auto el = queue.take();
        auto writer = el.get_writer();
        char* buf = nullptr;
        auto error = jvmti->GetSystemProperty(el.get_property().c_str(), &buf);
        if (JVMTI_ERROR_NONE == error) {
            std::string st{buf};
            jvmti->Deallocate(reinterpret_cast<unsigned char*> (buf));
            writer << "---\n";
            writer << st << "\n";
            writer << "---\n";
        } else {
            char* errbuf = nullptr;
            jvmti->GetErrorName(error, &errbuf);
            std::string st{errbuf};
            writer << "***\n";
            writer << st << "\n";
            writer << "***\n";
            jvmti->Deallocate(reinterpret_cast<unsigned char*> (errbuf));
        }
        // sending response to client
        writer->send();
    }
}

// required for worker init
static void JNICALL vm_init(jvmtiEnv *jvmti, JNIEnv *env, jthread thread) {
    (void) thread;
    // todo: error checking
    jvmti->RunAgentThread(alloc_thread(env), worker_fun, NULL, JVMTI_THREAD_MAX_PRIORITY);
}

void init_jvmti(JavaVM *jvm) {
    jvm->GetEnv((void **) &jvmti, JVMTI_VERSION);
    memset(&caps, 0, sizeof (caps));
    // not actually used
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
    memset(&callbacks, 0, sizeof (callbacks));
    callbacks.VMInit = &vm_init;
    // todo: errors checking
    jvmti->SetEventCallbacks(&callbacks, sizeof (callbacks));
    jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, NULL);
}

void start_http_server() {
    // start server
    server = new ph::streaming_server{1, TCP_PORT};
    server->add_method_specific_resource("GET", "/", [&](ph::request_ptr& req, pion::tcp::connection_ptr & conn) {
        // pion specific response writer creation
        auto finfun = std::bind(&pion::tcp::connection::finish, conn);
        auto writer = ph::response_writer::create(conn, *req, finfun);
        // reroute to worker
        queue.emplace(std::move(writer), req->get_resource().substr(1));
    });
    server->start();
}

} // namespace

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
    (void) options;
    (void) reserved;
    init_jvmti(jvm);
    std::cout << "I am loaded" << std::endl;
    start_http_server();
    std::cout << "Agent HTTP server started" << std::endl;
    return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
    (void) vm;
    server->stop(false);
    delete server;
    std::cout << "Agent HTTP server stopped" << std::endl;
}
