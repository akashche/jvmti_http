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
 * File:   JvmtiAccessor.cpp
 * Author: akashche
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
    // registered handlers map
    handlers.emplace("GetSystemProperty", handle_GetSystemProperty);
    handlers.emplace("GetStackTrace", handle_GetStackTrace);
    handlers.emplace("GetLiveThreadsCount", handle_GetLiveThreadsCount);
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
