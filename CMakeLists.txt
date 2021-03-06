# Copyright 2015, akashche at redhat.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required ( VERSION 2.8.12 )

# toolchain setup
if ( NOT DEFINED STATICLIB_TOOLCHAIN )
    if ( WIN32 )
        if ( CMAKE_SIZEOF_VOID_P EQUAL 4 )
            set ( STATICLIB_TOOLCHAIN "windows_i386_msvc" )
        else ( )
            set ( STATICLIB_TOOLCHAIN "windows_amd64_msvc" )
        endif ( )
    elseif ( APPLE )
        set ( STATICLIB_TOOLCHAIN "macosx_amd64_clang" )
    elseif ( UNIX )
        set ( STATICLIB_TOOLCHAIN "linux_amd64_gcc" )
    else ( )
        message ( FATAL_ERROR "Cannot determine toolchain to use, please specify [STATICLIB_TOOLCHAIN]" )
    endif ( )
endif ( )
message ( STATUS "Using toolchain: [${STATICLIB_TOOLCHAIN}]" )
set ( STATICLIB_CMAKE ${CMAKE_CURRENT_LIST_DIR}/cmake CACHE INTERNAL "" )    
set ( CMAKE_TOOLCHAIN_FILE ${STATICLIB_CMAKE}/toolchains/${STATICLIB_TOOLCHAIN}.cmake CACHE INTERNAL "" )

# project setup
project ( jvmti_http CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 0.1 )
# common include
include ( ${STATICLIB_CMAKE}/staticlibs_common.cmake )

# options
set ( JAVA_HOME "UNSPECIFIED" CACHE STRING "Path to JDK directory" )
set ( staticlib_httpserver_DISABLE_LOGGING ON CACHE BOOL "Disable logging support in httpserver" )
set ( staticlib_httpserver_USE_OPENSSL ON CACHE BOOL "Use OpenSSL lib for https" )

# dependencies
if ( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" ) # system dependencies implied on Linux
    staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/external_zlib )
    staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/external_openssl )
endif ( )
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/staticlib_config )
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/staticlib_io )
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/staticlib_endian )
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/staticlib_containers )
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/staticlib_utils )
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/staticlib_pimpl )
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/staticlib_httpserver )
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/deps/staticlib_unzip )

set ( ${PROJECT_NAME}_DEPS
        zlib
        openssl
        staticlib_config
        staticlib_io
        staticlib_endian
        staticlib_containers
        staticlib_utils
        staticlib_pimpl
        staticlib_unzip
        staticlib_httpserver )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# build steps
file ( GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp )
add_library ( ${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRC} )
target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/src 
        ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS}
        ${JAVA_HOME}/include
        ${JAVA_HOME}/include/linux
        ${JAVA_HOME}/include/win32 )
target_link_libraries ( ${PROJECT_NAME} 
        ${${PROJECT_NAME}_DEPS_PC_LIBRARIES} )
target_compile_options ( ${PROJECT_NAME} PRIVATE 
        ${${PROJECT_NAME}_DEPS_PC_CFLAGS_OTHER} )

# webapp target
add_custom_target( webapp
        COMMAND ${CMAKE_COMMAND} -E remove_directory webapp
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/webapp/public_html webapp
        COMMAND cd webapp && zip -qr webapp.zip *
        COMMAND ${CMAKE_COMMAND} -E rename ${PROJECT_BINARY_DIR}/webapp/webapp.zip ${PROJECT_BINARY_DIR}/bin/webapp.zip
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/webapp/certificates/cert.pem ${PROJECT_BINARY_DIR}/bin/cert.pem
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR} )
