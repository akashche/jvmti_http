/* 
 * File:   JvmtiAccessor.hpp
 * Author: netbeans
 *
 * Created on July 10, 2015, 9:13 PM
 */

#ifndef JVMTIACCESSOR_HPP
#define	JVMTIACCESSOR_HPP

#include <string>

#include <jvmti.h>

namespace jvmti_http {

class JvmtiAccessor {
public:
    JvmtiAccessor(const JvmtiAccessor&) = delete;
    JvmtiAccessor& operator=(const JvmtiAccessor&) = delete;
   
    JvmtiAccessor(jvmtiEnv* jvmti);
    
    std::string process_query(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& query);

};

} // namespace

#endif	/* JVMTIACCESSOR_HPP */

