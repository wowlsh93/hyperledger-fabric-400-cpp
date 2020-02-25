//
// Created by hama on 20. 2. 25..
//

#ifndef HYPERLEDGER_FABRIC_400_CPP_STRING_FORMAT_H
#define HYPERLEDGER_FABRIC_400_CPP_STRING_FORMAT_H

#endif //HYPERLEDGER_FABRIC_400_CPP_STRING_FORMAT_H


#include <iostream>
#include <vector>
#include <string>
#include <cstdarg>
#include <cstring>
#include <memory>

namespace hama {

    template<typename ... Args>
    std::string string_format(const std::string& format, Args ... args) {

        size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }
}
