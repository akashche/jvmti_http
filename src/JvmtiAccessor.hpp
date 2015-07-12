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
 * File:   JvmtiAccessor.hpp
 * Author: akashche
 *
 * Created on July 10, 2015, 9:13 PM
 */

#ifndef JVMTI_HTTP_JVMTIACCESSOR_HPP
#define	JVMTI_HTTP_JVMTIACCESSOR_HPP

#include <string>
#include <unordered_map>
#include <functional>

#include <jvmti.h>

namespace jvmti_http {

class JvmtiAccessor {
    std::unordered_map<std::string, std::function<std::string(jvmtiEnv*, JNIEnv*, const std::string&)>> handlers;
    
public:
    JvmtiAccessor(const JvmtiAccessor&) = delete;
    JvmtiAccessor& operator=(const JvmtiAccessor&) = delete;
   
    JvmtiAccessor();
    
    std::string process_query(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& query);    
};

} // namespace

#endif	/* JVMTI_HTTP_JVMTIACCESSOR_HPP */

