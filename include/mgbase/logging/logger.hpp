
#pragma once

#include <cstdio>
#include <functional>
#include <iostream>

#include <cppformat/format.h>

namespace mgbase {

#define MGBASE_LOG_LEVEL_DEBUG 0
#define MGBASE_LOG_LEVEL_FATAL 1000

class logger {
public:
    typedef mgbase::uint64_t log_level_t;
    
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
    
    static log_level_t get_log_level() {
        static const log_level_t result = get_log_level_from_env();
        return result;
    }

private:
    static log_level_t get_log_level_from_env() {
        const char* const value = std::getenv("MGBASE_LOG_LEVEL");
        if (value == MGBASE_NULLPTR)
            return MGBASE_LOG_LEVEL_FATAL;
        
        return static_cast<log_level_t>(std::atoi(value));
    }
    
    // TODO: Singleton
    static state_callback_type& get_state_callback() {
        static state_callback_type callback = MGBASE_NULLPTR;
        return callback;
    }
};


#ifndef MGBASE_ENABLE_LOG
    #if MGBASE_DEBUG
        #define MGBASE_ENABLE_LOG
    #else
        //#define MGBASE_ENABLE_LOG // Disabled
    #endif
#endif

namespace detail {

// This function is never defined.
void logger_not_defined(...);

}

#ifdef MGBASE_ENABLE_LOG
    #define MGBASE_LOG_DEBUG(...)  \
        if (::mgbase::logger::get_log_level() <= MGBASE_LOG_LEVEL_DEBUG) { \
            std::cout << ::mgbase::logger::get_state(); \
            fmt::print(__VA_ARGS__); \
            std::cout << std::endl; \
        }
#else
    // Do a static type check (but do nothing in run-time)
    #define MGBASE_LOG_DEBUG(...)   \
        if (false) { mgbase::detail::logger_not_defined(__VA_ARGS__); }
#endif

}

