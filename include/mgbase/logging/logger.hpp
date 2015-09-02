
#pragma once

#include <cstdio>
#include <functional>
#include <iostream>

#include <cppformat/format.h>

namespace mgbase {

class logger {
public:
    typedef std::string (*state_callback_type)();
    
    /*template <typename... Args>
    static void debug(const char* msg, Args&&... args) {
        fmt::MemoryWriter w;
        w.write(std::string("DEBUG: ") + get_state_callback()() + " " + msg, args...);
        std::cout << w.str() << std::endl;
    }*/
    
    static void set_state_callback(state_callback_type callback) {
        get_state_callback() = callback;
    }
    
    static std::string get_state() {
        state_callback_type callback = get_state_callback();
        if (callback != MGBASE_NULLPTR)
            return callback();
        else
            return std::string();
    }

private:
    // TODO: Singleton
    static state_callback_type& get_state_callback() {
        static state_callback_type callback = MGBASE_NULLPTR;
        return callback;
    }
};

#define MGBASE_LOG_LEVEL_DEBUG 0

#ifndef MGBASE_LOG_LEVEL
    #define MGBASE_LOG_LEVEL  MGBASE_LOG_LEVEL_DEBUG
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#if MGBASE_LOG_LEVEL <= MGBASE_LOG_LEVEL_DEBUG
    #define MGBASE_LOG_DEBUG(format, ...)  fmt::print("{}" format, ::mgbase::logger::get_state(), ## __VA_ARGS__), std::cout << std::endl
#else
    #define MGBASE_LOG_DEBUG(format, ...)
#endif

#pragma GCC diagnostic pop

}

