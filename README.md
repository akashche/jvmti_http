HTTP access to JVMTI
====================

This project contains an implementation of HTTP server embedded into
JVMTI agent and a stub logic for accessing JVMTI API remotely over HTTP.
Currently it only allows remote access to JVM system properties.

Build and run steps (was tested on Fedora 22):

    git clone ...
    cd jvmti_http
    git submodule update --init --recursive
    mkdir build
    cd build
    cmake ..
    make
    javac -d . ../test/App.java && java -agentpath:`pwd`/lib/libjvmti_http.so App

HTTP server will be started on port 8080 and will return value of the system property name
specified in URL, examples:

    curl http://127.0.0.1:8080/java.vm.name
    ---
    OpenJDK 64-Bit Server VM
    ---

    curl http://127.0.0.1:8080/java.vm.version
    ---
    25.45-b02
    ---

    curl http://127.0.0.1:8080/java.library.path 
    ---
    /usr/java/packages/lib/amd64:/usr/lib64:/lib64:/lib:/usr/lib
    ---

    curl http://127.0.0.1:8080/java.home
    ---
    /usr/lib/jvm/java-1.8.0-openjdk-1.8.0.45-40.b14.fc22.x86_64/jre
    ---
