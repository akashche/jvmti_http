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
 * File:   handlers.hpp
 * Author: akashche
 *
 * Created on July 12, 2015, 6:34 PM
 */

#ifndef JVMTI_HTTP_HANDLERS_HPP
#define	JVMTI_HTTP_HANDLERS_HPP

#include <string>
#include <stdexcept>
#include <jvmti.h>

#include "staticlib/utils/tracemsg.hpp"

// handler functions

std::string handle_GetSystemProperty(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& input);

std::string handle_GetStackTrace(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& input);

std::string handle_GetLiveThreadsCount(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& input);

// utility functions

std::string to_error_message(jvmtiEnv *jvmti, jvmtiError error);

#endif	/* JVMTI_HTTP_HANDLERS_HPP */

