/* 
 * File:   GetSystemProperty.cpp
 * Author: netbeans
 *
 * Created on July 12, 2015, 6:41 PM
 */

#include "handlers.hpp"

std::string handle_GetSystemProperty(jvmtiEnv* jvmti, JNIEnv* /* jni */, const std::string& input) {
    char* buf = nullptr;
    auto error = jvmti->GetSystemProperty(input.c_str(), &buf);
    auto error_str = to_error_message(jvmti, error);
    if (!error_str.empty()) {
        throw std::runtime_error(TRACEMSG(error_str));
    }
    return std::string{buf};
}
