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
 * File:   GetLiveThreadsCount.cpp
 * Author: alex
 *
 * Created on July 31, 2015, 2:51 PM
 */

#include "handlers.hpp"

// it may be better to get such information through 
// "ThreadStart" and "ThreadEnd" events instead of this direct method
std::string handle_GetLiveThreadsCount(jvmtiEnv* jvmti, JNIEnv* jni, const std::string&) {
    jthread* threads = nullptr;
    jint count = -1;
    auto error = jvmti->GetAllThreads(&count, &threads);
    auto error_str = to_error_message(jvmti, error);
    if (!error_str.empty()) {
        throw std::runtime_error(TRACEMSG(error_str));
    }
    // release threads
    for (jint i = 0; i < count; i++) {
        jni->DeleteLocalRef(threads[i]);
    }
    jvmti->Deallocate(reinterpret_cast<unsigned char*> (threads));
    return std::to_string(count);
}
