/* 
 * File:   JvmtiAccessor.hpp
 * Author: netbeans
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

