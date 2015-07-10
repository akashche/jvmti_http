/* 
 * File:   JvmtiAccessor.cpp
 * Author: netbeans
 * 
 * Created on July 10, 2015, 9:13 PM
 */

#include <string>

#include <jvmti.h>

#include "JvmtiAccessor.hpp"

namespace jvmti_http {

JvmtiAccessor::JvmtiAccessor(jvmtiEnv* /* jvmti */) { }

std::string JvmtiAccessor::process_query(jvmtiEnv* jvmti, JNIEnv* /* jni */, const std::string& query) {
    char* buf = nullptr;
    auto error = jvmti->GetSystemProperty(query.c_str(), &buf);
    if (JVMTI_ERROR_NONE == error) {
        std::string st{buf};
        jvmti->Deallocate(reinterpret_cast<unsigned char*> (buf));
        return st;
    } else {
        char* errbuf = nullptr;
        jvmti->GetErrorName(error, &errbuf);
        std::string st{errbuf};        
        jvmti->Deallocate(reinterpret_cast<unsigned char*> (errbuf));
        return st;
    }
}

} // namespace

