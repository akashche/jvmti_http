/* 
 * File:   JvmtiHttpQuery.hpp
 * Author: netbeans
 *
 * Created on July 10, 2015, 7:27 PM
 */

#ifndef JVMTI_HTTP_JVMTIHTTPQUERY_HPP
#define	JVMTI_HTTP_JVMTIHTTPQUERY_HPP

#include <string>

#include "pion/http/response_writer.hpp"

namespace jvmti_http {

class JvmtiHttpQuery {
    pion::http::response_writer_ptr writer;
    std::string property;
    
public:
    JvmtiHttpQuery(const JvmtiHttpQuery&) = delete;
    JvmtiHttpQuery& operator=(const JvmtiHttpQuery&) = delete;

    JvmtiHttpQuery(JvmtiHttpQuery&& other);

    JvmtiHttpQuery& operator=(JvmtiHttpQuery&& other);

    JvmtiHttpQuery();
    
    JvmtiHttpQuery(pion::http::response_writer_ptr writer, std::string property);

    pion::http::response_writer_ptr& get_writer();

    std::string& get_property();
};

} // namespace

#endif	/* JVMTI_HTTP_JVMTIHTTPQUERY_HPP */

