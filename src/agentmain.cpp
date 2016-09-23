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
 * File:   main.cpp
 * Author: akashche
 *
 * Created on July 1, 2015, 1:44 PM
 */

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <jvmti.h>

#include "staticlib/utils.hpp"

#include "HttpServer.hpp"
#include "JvmtiAccessor.hpp"
#include "JvmtiHttpException.hpp"

namespace { // anonymous

namespace su = staticlib::utils;
namespace sh = staticlib::httpserver;

// http server
jvmti_http::HttpServer* GLOBAL_SERVER;

void check_error(jvmtiEnv *jvmti, jvmtiError error) {
    if (JVMTI_ERROR_NONE != error) {
        char* errbuf = nullptr;
        jvmti->GetErrorName(error, &errbuf);
        std::string errstr{errbuf};
        jvmti->Deallocate(reinterpret_cast<unsigned char*> (errbuf));
        throw jvmti_http::JvmtiHttpException(TRACEMSG(errstr));
    }
}

// helper function for thread init
jthread alloc_thread(JNIEnv* env) {
    jclass thrClass = env->FindClass("java/lang/Thread");
    if (nullptr == thrClass) {
        throw jvmti_http::JvmtiHttpException(TRACEMSG("Cannot find Thread class\n"));
    }
    jmethodID cid = env->GetMethodID(thrClass, "<init>", "()V");
    if (nullptr == cid) {
        throw jvmti_http::JvmtiHttpException(TRACEMSG("Cannot find Thread constructor method\n"));
    }
    jthread res = env->NewObject(thrClass, cid);
    if (nullptr == res) {
        throw jvmti_http::JvmtiHttpException(TRACEMSG("Cannot create new Thread object\n"));
    }
    return res;
}

// required for worker init
static void JNICALL vm_init(jvmtiEnv *jvmti, JNIEnv *env, jthread /* thread */) {
    auto th = alloc_thread(env);
    auto error = jvmti->RunAgentThread(th, jvmti_http::HttpServer::jvmti_callback, GLOBAL_SERVER, JVMTI_THREAD_NORM_PRIORITY);
    check_error(jvmti, error);
}

jvmtiEnv* init_jvmti(JavaVM *jvm) {
    jvmtiEnv* jvmti;
    jvm->GetEnv((void **) &jvmti, JVMTI_VERSION);
    // http://docs.oracle.com/javase/7/docs/platform/jvmti/jvmti.html#jvmtiCapabilities
    jvmtiCapabilities caps;
    memset(&caps, 0, sizeof (caps));
    auto error = jvmti->AddCapabilities(&caps);
    check_error(jvmti, error);
    return jvmti;
}

void add_jvmti_callback(jvmtiEnv* jvmti) {
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof (callbacks));
    callbacks.VMInit = &vm_init;
    auto errorcb = jvmti->SetEventCallbacks(&callbacks, sizeof (callbacks));
    check_error(jvmti, errorcb);
    auto errorev = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, NULL);
    check_error(jvmti, errorev);
}

size_t check_opts(const std::string& options) {
    auto pos = options.find(',');
    if (std::string::npos == pos || pos + 1 >= options.length()) {
        throw jvmti_http::JvmtiHttpException(TRACEMSG(
            "Invalid agent options, must be =<port>,<webapp_path>"));
    }
    return pos;
}

void check_webapp_path(const std::string& webapp_zip_path, const std::string& cert_path) {
    std::ifstream zip{webapp_zip_path};
    if (!zip.good()) {
        throw jvmti_http::JvmtiHttpException(TRACEMSG("Zip file not found: [" + webapp_zip_path + "]"));
    }
    std::ifstream cert{cert_path};
    if (!cert.good()) {
        throw jvmti_http::JvmtiHttpException(TRACEMSG("Server certificate not found: [" + cert_path + "]"));
    }
}

} // namespace

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* jvm, char* options, void* /* reserved */) {
    try {
        jvmtiEnv * jvmti = init_jvmti(jvm);
        std::string optstr{options};
        auto comma = check_opts(optstr);
        uint16_t port = su::parse_uint16(optstr.substr(0, comma));
        // config parameters may be passed to accessor
        auto ja = new jvmti_http::JvmtiAccessor();
        auto webapp_path = optstr.substr(comma + 1);
        auto webapp_zip_path = webapp_path + "/webapp.zip";
        auto cert_path = webapp_path + "/cert.pem";
        check_webapp_path(webapp_zip_path, cert_path);
        GLOBAL_SERVER = new jvmti_http::HttpServer(port, ja, webapp_zip_path, cert_path);
        add_jvmti_callback(jvmti);
        std::cout << "Agent HTTP server started on port: [" << port << "]" << std::endl;
        return JNI_OK;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return JNI_ERR;
    }
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM* /* vm */) {
    GLOBAL_SERVER->shutdown();
    delete GLOBAL_SERVER;
    std::cout << "Agent HTTP server stopped" << std::endl;
}
