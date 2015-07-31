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
 * File:   ZipResource.hpp
 * Author: alex
 *
 * Created on July 31, 2015, 3:16 PM
 */

#ifndef JVMTI_HTTP_ZIPRESOURCE_HPP
#define	JVMTI_HTTP_ZIPRESOURCE_HPP

#include "pion/tcp/connection.hpp"
#include "pion/http/request.hpp"

namespace jvmti_http {

class ZipResource {
    std::string file_path;
    std::string url_prefix;
    
public:
    ZipResource(const std::string& file_path, const std::string& url_prefix);
    
    void handle(pion::http::request_ptr& req, pion::tcp::connection_ptr& conn);

};

} // namespace

#endif	/* JVMTI_HTTP_ZIPRESOURCE_HPP */

