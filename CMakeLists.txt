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
set ( CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/toolchains/${STATICLIB_TOOLCHAIN}.cmake CACHE INTERNAL "" )

# project setup
project ( jvmti_http CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 0.1 )
# common include
include ( ${CMAKE_CURRENT_LIST_DIR}/cmake/staticlibs_common.cmake )

# dependencies

staticlib_add_subdirectory ( deps staticlib_utils )
staticlib_add_subdirectory ( deps staticlib_containers )
SET ( STATICLIB_PION_DISABLE_LOGGING ON CACHE BOOL "")
staticlib_add_subdirectory ( deps staticlib_httpserver )

# build steps
file ( GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp )
add_library ( ${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRC} )
target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/src 
        ${staticlib_utils_INCLUDES}
        ${staticlib_containers_INCLUDES}
        ${staticlib_httpserver_INCLUDES}
        /usr/lib/jvm/java-openjdk/include 
        /usr/lib/jvm/java-openjdk/include/linux )
target_link_libraries ( ${PROJECT_NAME} 
        staticlib_utils 
        staticlib_httpserver 
        pthread )
target_compile_definitions ( ${PROJECT_NAME} PRIVATE 
        -DASIO_STANDALONE 
        -DASIO_HAS_STD_CHRONO )
