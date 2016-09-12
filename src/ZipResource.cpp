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
#include <array>
#include <functional>

#include "staticlib/config.hpp"
#include "staticlib/httpserver.hpp"
#include "staticlib/io.hpp"
#include "staticlib/unzip.hpp"
#include "staticlib/utils.hpp"

#include "JvmtiHttpException.hpp"
#include "ZipResource.hpp"

namespace jvmti_http {

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;
namespace si = staticlib::io;
namespace sz = staticlib::unzip;
namespace su = staticlib::utils;

namespace detail {


class ResponseStreamSender : public std::enable_shared_from_this<ResponseStreamSender> {
    sh::http_response_writer_ptr writer;
    std::unique_ptr<std::istream> st;

    std::array<char, 4096> buf;

public:

    ResponseStreamSender(sh::http_response_writer_ptr&& writer, std::unique_ptr<std::istream>&& st) :
    writer(std::move(writer)),
    st(std::move(st)) { }

    void send() {
        asio::error_code ec{};
        handle_write(ec, 0);
    }

    void handle_write(const asio::error_code& ec, size_t /* bytes_written */) {
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
url_prefix(url_prefix),
idx(new sz::UnzipFileIndex(file_path)) { }

void ZipResource::handle(sh::http_request_ptr& req, sh::tcp_connection_ptr& conn) {
    auto resp = sh::http_response_writer::create(conn, req);
    std::string url_path = std::string(req->get_resource(), url_prefix.length());
    sz::FileEntry en = idx->find_zip_entry(url_path);
    if (!en.is_empty()) {
        auto stream_ptr = sz::open_zip_entry(*idx, url_path);
        auto sender = std::make_shared<detail::ResponseStreamSender>(std::move(resp), std::move(stream_ptr));
        // todo
        //            set_resp_headers(url_path, resp->get_response());
        sender->send();
    } else {
        resp->get_response().set_status_code(sh::http_request::RESPONSE_CODE_NOT_FOUND);
        resp->get_response().set_status_message(sh::http_request::RESPONSE_MESSAGE_NOT_FOUND);
        resp << sh::http_request::RESPONSE_CODE_NOT_FOUND << " "
                << sh::http_request::RESPONSE_MESSAGE_NOT_FOUND << ":"
                << " [" << url_path << "]\n";
        resp->send();
    }
}

//    void set_resp_headers(const std::string& url_path, sh::http_response& resp) {
//        std::string ct{"application/octet-stream"};
//        for (const auto& mi : conf->mimeTypes) {
//            if (su::ends_with(url_path, mi.extension)) {
//                ct = mi.mime;
//                break;
//            }
//        }
//        resp.change_header("Content-Type", ct);
//        // set caching
//        resp.change_header("Cache-Control", "max-age=" + sc::to_string(conf->cacheMaxAgeSeconds) + ", public");
//    }

} // namespace
