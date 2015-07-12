/* 
 * File:   GetStackTrace.cpp
 * Author: netbeans
 *
 * Created on July 12, 2015, 7:09 PM
 */

#include <array>

#include "handlers.hpp"

std::string handle_GetStackTrace(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& input) {
    // get all threads
    jthread* threads = nullptr;
    jint count = -1;
    auto error = jvmti->GetAllThreads(&count, &threads);
    auto error_str = to_error_message(jvmti, error);
    if (!error_str.empty()) {
        throw std::runtime_error(TRACEMSG(error_str));
    }
    // find thread with requested name
    auto clazz = jni->FindClass("java/lang/Thread");
    auto method = jni->GetMethodID(clazz, "getName", "()Ljava/lang/String;");
    bool found = false;
    std::string res{};
    for (jint i = 0; i < count; i++) {
        auto name_java_obj = jni->CallObjectMethod(threads[i], method);
        auto name_java = static_cast<jstring>(name_java_obj);
        auto name = jni->GetStringUTFChars(name_java, nullptr);        
        if (input == name) { 
            found = true;
            // requested thread found - get its stacktrace
            jint frames_count = -1;
            std::array<jvmtiFrameInfo, 1024> frames;
            auto error_gst = jvmti->GetStackTrace(threads[i], 0, 1024, frames.data(), &frames_count);
            auto error_gst_str = to_error_message(jvmti, error_gst);            
            if (!error_gst_str.empty()) {
                res = error_gst_str;
            }
            // format stack frames as a list of methods
            for (jint i = 0; i < frames_count; i++) {
                char* method_name;
                char* signature;
                // should not fail
                jvmti->GetMethodName(frames[i].method, &method_name, &signature, nullptr);
                res += method_name;
                res += signature;
                if (-1 == frames[i].location) {
                    res += " [native]";
                }
                res += '\n';
                jvmti->Deallocate(reinterpret_cast<unsigned char*>(method_name));
                jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
            }
        }
        jni->ReleaseStringUTFChars(name_java, name);
        if (found) {
            break;
        }
    }
    if (!found) {
        res = std::string("Thread not found: [" + input + "]\n");
    }
    
    // release threads
    for (jint i = 0; i < count; i++) {
        jni->DeleteLocalRef(threads[i]);
    }
    jvmti->Deallocate(reinterpret_cast<unsigned char*>(threads));
    
    return res;
    
}
