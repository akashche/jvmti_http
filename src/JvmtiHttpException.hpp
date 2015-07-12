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
 * File:   JvmtiHttpException.hpp
 * Author: akashche
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

