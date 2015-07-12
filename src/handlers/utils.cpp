/* 
 * File:   utils.cpp
 * Author: netbeans
 *
 * Created on July 12, 2015, 6:45 PM
 */

#include "handlers.hpp"

std::string to_error_message(jvmtiEnv *jvmti, jvmtiError error) {
    if (JVMTI_ERROR_NONE == error) {
        return std::string();
    }
    char* errbuf = nullptr;
    jvmti->GetErrorName(error, &errbuf);
    std::string res{errbuf};
    jvmti->Deallocate(reinterpret_cast<unsigned char*> (errbuf));
    return res;
}

