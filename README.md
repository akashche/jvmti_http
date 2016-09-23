HTTP access to JVMTI
====================

This project contains an implementation of HTTP server embedded into
[JVMTI agent](http://docs.oracle.com/javase/7/docs/platform/jvmti/jvmti.html) that allows 
to access JVMTI-related functionality remotely over HTTP.

Access to JVMTI is implemented using [handler functions](https://github.com/akashche/jvmti_http/tree/master/src/handlers).
Each handler function takes an input string from HTTP URL and returns result string that is sent to client in HTTP response.
Handler name is specified as a first part of the URL.

The following handlers are implemented:

 * `/jvmti/GetSystemProperty/<property_name>` - returns value for the specified system property using
 [GetSystemProperty](http://docs.oracle.com/javase/7/docs/platform/jvmti/jvmti.html#GetSystemProperty)
function
 * `/jvmti/GetStackTrace/<thread_name>` - returns a stack trace for the specified thread using
[GetStackTrace](http://docs.oracle.com/javase/7/docs/platform/jvmti/jvmti.html#GetStackTrace) function
 * `/jvmti/GetLiveThreadsCount/` - returns a number of live threads in JVM using 
[GetAllThreads](http://docs.oracle.com/javase/7/docs/platform/jvmti/jvmti.html#GetAllThreads) function

How to build and run
--------------------

Build and run steps tested on Windows and Linux and should work (with minor changes) 
on other platforms supported by OpenJDK.

    git clone --recursive https://github.com/akashche/jvmti_http.git
    cd jvmti_http
    mkdir build
    cd build
    cmake .. -DJAVA_HOME=<path/to/jdk>
    cmake --build .
    cmake --build . --target webapp
    javac -d . ../test/App.java && java -agentpath:bin/libjvmti_http.so=8443,bin App

HTTP server will be started on the specified port (8443 in this example). In this example
server uses self-signed HTTPS certificate. Example can be easily extended to use different server certificate
or to require clients to use HTTPS-client certificates for authentication.

Usage examples with cURL:

    curl -k https://127.0.0.1:8443/jvmti/GetSystemProperty/java.home
    curl -k https://127.0.0.1:8443/jvmti/GetSystemProperty/java.vm.version
    curl -k https://127.0.0.1:8443/jvmti/GetStackTrace/main

Web application example
-----------------------

[Example web app](https://github.com/akashche/dynamic_bar_chart_example) is included with this
project as a git submodule. Web app is bundled into a ZIP file and web content is served using the
same embedded HTTP server. Web app access [link](https://127.0.0.1:8443/webapp/index.html):

    https://127.0.0.1:8443/webapp/index.html

How to add new handlers
-----------------------

 * implement handler function with the following signature:

    `std::string handle_Something(jvmtiEnv* jvmti, JNIEnv* jni, const std::string& input);`

 * put this function into separate source file into [handlers directory](https://github.com/akashche/jvmti_http/tree/master/src/handlers)

 * add function declaration to [handlers header](https://github.com/akashche/jvmti_http/tree/master/src/handlers.hpp#L33)

 * add function pointer to the [handlers map in JvmtiAccessor class](https://github.com/akashche/jvmti_http/tree/master/src/JvmtiAccessor.cpp#L36)

 * run `touch ../CMakeLists.txt` to register new source file (cmake re-run is not required)

 * build and run the project and test access to your new handler using cURL or web-browser


License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0)

Changelog
---------

**2016-09-23**

 * dependencies cleanup
 * windows support

**2015-08-04**

 * web app example added

**2015-07-12**

 * initial public version
