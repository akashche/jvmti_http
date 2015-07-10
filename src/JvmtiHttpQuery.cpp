/* 
 * File:   JvmtiHttpQuery.cpp
 * Author: netbeans
 * 
 * Created on July 10, 2015, 7:27 PM
 */

#include "JvmtiHttpQuery.hpp"

namespace jvmti_http {

JvmtiHttpQuery::JvmtiHttpQuery(JvmtiHttpQuery&& other) :
writer(std::move(other.writer)),
property(std::move(other.property)) { }

JvmtiHttpQuery& JvmtiHttpQuery::operator=(JvmtiHttpQuery&& other) {
    this->writer = std::move(other.writer);
    this->property = std::move(other.property);
    return *this;
}

JvmtiHttpQuery::JvmtiHttpQuery() { }

JvmtiHttpQuery::JvmtiHttpQuery(pion::http::response_writer_ptr writer, std::string property) :
writer(std::move(writer)),
property(std::move(property)) { }

pion::http::response_writer_ptr& JvmtiHttpQuery::get_writer() {
    return writer;
}

std::string& JvmtiHttpQuery::get_property() {
    return property;
}

} // namespace