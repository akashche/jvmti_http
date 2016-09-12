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
 * File:   ZipResource.cpp
 * Author: alex
 * 
 * Created on July 31, 2015, 3:16 PM
 */

#include <string>
#include <memory>
#include <istream>
#include <mutex>
#include <array>
#include <functional>

#include "unzip.h"

#include "staticlib/config.hpp"
#include "staticlib/httpserver.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"

#include "JvmtiHttpException.hpp"
#include "ZipResource.hpp"

namespace jvmti_http {

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;
namespace si = staticlib::io;
namespace su = staticlib::utils;

namespace detail {

std::string extract_path(const std::string& resource, const std::string& url_prefix) {
    auto comres = resource.compare(0, url_prefix.length(), url_prefix);
    if (0 == comres && resource.length() > url_prefix.length()) {
        return resource.substr(url_prefix.length() + 1);
    } else {
        return resource;
    }
}

class UnzipFileCloser {
public:
    void operator()(unzFile file) {
        unzClose(file);
    }
};

class UnzipEntry {
    std::unique_ptr<void, UnzipFileCloser> src;

public:
    UnzipEntry(std::unique_ptr<void, UnzipFileCloser>&& src) :
    src(std::move(src)) { }

    std::streamsize read(char* s, std::streamsize n) {
        auto uf = src.get();
        auto res = unzReadCurrentFile(uf, s, n);
        if (res > 0) {
            return res;
        } else if (0 == res) {
            return std::char_traits<char>::eof();
        } else {
            throw JvmtiHttpException(TRACEMSG(
                    "Resource read error, code: [" + sc::to_string(res) + "]"));
        }
    }
};

class ResponseStreamSender : public std::enable_shared_from_this<ResponseStreamSender> {
    sh::http_response_writer_ptr writer;
    std::unique_ptr<std::istream> st;

    std::array<char, 8192> buf;
    std::mutex mutex;

public:

    ResponseStreamSender(sh::http_response_writer_ptr&& writer, std::unique_ptr<std::istream>&& st) :
    writer(std::move(writer)),
    st(std::move(st)) { }

    void send() {
        asio::error_code ec{};
        handle_write(ec, 0);
    }

    void handle_write(const asio::error_code& ec, size_t /* bytes_written */) {
        std::lock_guard<std::mutex> lock{mutex};
        if (!ec) {
            auto src = si::streambuf_source(st->rdbuf());
            size_t len = si::read_all(src, buf.data(), buf.size());
            if (len > 0) {
                writer->clear();
                writer->write_no_copy(buf.data(), len);
            }
            if (buf.size() == len) {
                writer->send_chunk(std::bind(&ResponseStreamSender::handle_write, shared_from_this(),
                        std::placeholders::_1, std::placeholders::_2));
            } else {
                writer->send_final_chunk();
            }
        } else {
            // make sure it will get closed
            writer->get_connection()->set_lifecycle(sh::tcp_connection::LIFECYCLE_CLOSE);
        }
    }
};

} // namespace

ZipResource::ZipResource(const std::string& file_path, const std::string& url_prefix) :
file_path(file_path),
url_prefix(url_prefix) { }

void ZipResource::handle(sh::http_request_ptr& req, sh::tcp_connection_ptr& conn) {
    auto writer = sh::http_response_writer::create(conn, req);
    
    auto path = detail::extract_path(req->get_resource(), url_prefix);

    std::unique_ptr<void, detail::UnzipFileCloser> file{unzOpen(file_path.c_str()), detail::UnzipFileCloser{}};
    if (!file.get()) throw JvmtiHttpException(TRACEMSG(std::string{} +
            "Error opening zip file: [" + file_path + "]"));

    auto found = unzLocateFile(file.get(), path.c_str(), 1);
    if (UNZ_OK != found) {
        writer->get_response().set_status_code(404);
        writer->get_response().set_status_message("Not Found");
        writer << "Not Found";
        writer->send();
        return;
    }
    auto opened = unzOpenCurrentFile(file.get());
    if (UNZ_OK != opened) {
        throw JvmtiHttpException(TRACEMSG(std::string{} +
                "Error opening resource: [" + path + "] zip file: [" + file_path + "]"));
    }

    // prepare input stream for sender ensuring lifetime for open zip entry
    auto st = si::make_source_istream_ptr(detail::UnzipEntry(std::move(file)));
    auto sender = std::make_shared<detail::ResponseStreamSender>(std::move(writer), std::move(st));
    sender->send();
}

} // namespace
