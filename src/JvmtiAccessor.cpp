/* 
 * File:   JvmtiAccessor.cpp
 * Author: netbeans
 * 
 * Created on July 10, 2015, 9:13 PM
 */

#include <string>

#include <jvmti.h>

#include "staticlib/utils/tracemsg.hpp"

#include "handlers.hpp"
#include "JvmtiHttpException.hpp"
#include "JvmtiAccessor.hpp"

namespace jvmti_http {

JvmtiAccessor::JvmtiAccessor() {
    handlers.emplace("GetSystemProperty", handle_GetSystemProperty);
    handlers.emplace("GetStackTrace", handle_GetStackTrace);
}

std::string JvmtiAccessor::process_query(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& query) {
    auto slash_ind = query.find_first_of("/");
    if (std::string::npos != slash_ind && query.length() > slash_ind) {
        std::string hname = query.substr(0, slash_ind);
        std::string input = query.substr(slash_ind + 1);
        auto tu = handlers.find(hname);
        if (handlers.end() != tu) {
            try {
                return tu->second(jvmti, jni, input);
            } catch (const std::exception& e) {
                throw JvmtiHttpException(TRACEMSG(std::string(e.what()) + "\n" +
                        "JvmtiHttpException: handler throw exception for query: [" + query + "]"));
            }
        } else {
            throw JvmtiHttpException(TRACEMSG(std::string("Invalid handler name: [") + hname + "]"));
        }
    } else {
        throw JvmtiHttpException(TRACEMSG(std::string("Invalid query: [") + query + "]"));
    }
}

} // namespace
