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
set ( STATICLIB_TOOLCHAIN linux_amd64_gcc CACHE STRING "toolchain triplet" )
set ( STATICLIB_CMAKE ${CMAKE_CURRENT_LIST_DIR}/cmake CACHE INTERNAL "" )    
set ( CMAKE_TOOLCHAIN_FILE ${STATICLIB_CMAKE}/toolchains/${STATICLIB_TOOLCHAIN}.cmake CACHE INTERNAL "" )

# project setup
project ( jvmti_http CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 0.1 )
# common include
include ( ${STATICLIB_CMAKE}/staticlibs_common.cmake )

# options
set ( JAVA_HOME "/usr/lib/jvm/java-openjdk" CACHE STRING "JDK directory" )
#set ( staticlib_httpserver_DISABLE_LOGGING ON CACHE BOOL "Disable logging support in httpserver" )
set ( staticlib_httpserver_USE_OPENSSL ON CACHE BOOL "Use OpenSSL lib for https" )

# dependencies
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
        ${JAVA_HOME}/include/linux )
target_link_libraries ( ${PROJECT_NAME} 
        ${${PROJECT_NAME}_DEPS_PC_LIBRARIES} )
target_compile_options ( ${PROJECT_NAME} PRIVATE 
        ${${PROJECT_NAME}_DEPS_PC_CFLAGS_OTHER} )

# webapp target
add_custom_target( webapp
        COMMAND ${CMAKE_MAKE_PROGRAM} -f ${CMAKE_CURRENT_LIST_DIR}/webapp/Makefile
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/webapp )
