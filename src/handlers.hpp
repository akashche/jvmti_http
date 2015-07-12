/* 
 * File:   handlers.hpp
 * Author: netbeans
 *
 * Created on July 12, 2015, 6:34 PM
 */

#ifndef JVMTI_HTTP_HANDLERS_HPP
#define	JVMTI_HTTP_HANDLERS_HPP

#include <string>
#include <stdexcept>
#include <jvmti.h>

#include "JvmtiHttpException.hpp"
#include "staticlib/utils/tracemsg.hpp"

std::string handle_GetSystemProperty(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& input);

std::string handle_GetStackTrace(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& input);

// utility functions

std::string to_error_message(jvmtiEnv *jvmti, jvmtiError error);

#endif	/* JVMTI_HTTP_HANDLERS_HPP */

