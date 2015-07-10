/* 
 * File:   JvmtiHttpException.hpp
 * Author: netbeans
 *
 * Created on July 10, 2015, 7:43 PM
 */

#ifndef JVMTIHTTPEXCEPTION_HPP
#define	JVMTIHTTPEXCEPTION_HPP

#include "staticlib/utils/BaseException.hpp"

namespace jvmti_http {

/**
 * Module specific exception
 */
class JvmtiHttpException : public staticlib::utils::BaseException {
public:
    /**
     * Default constructor
     */
    JvmtiHttpException() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    JvmtiHttpException(const std::string& msg) :
    staticlib::utils::BaseException(msg) { }

};

} // namespace

#endif	/* JVMTIHTTPEXCEPTION_HPP */

