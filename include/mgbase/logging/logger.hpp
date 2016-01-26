
#pragma once

#include <cstdio>
#include <functional>
#include <iostream>

#include <mgbase/external/cppformat.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>

namespace mgbase {

#define MGBASE_LOG_LEVEL_VERBOSE    0
#define MGBASE_LOG_LEVEL_DEBUG      1
#define MGBASE_LOG_LEVEL_INFO       10
#define MGBASE_LOG_LEVEL_WARN       100
#define MGBASE_LOG_LEVEL_FATAL      1000

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
    
    // TODO: Singleton
    static mgbase::spinlock& get_lock() {
        static mgbase::spinlock lc;
        return lc;
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

} // namespace detail

#ifdef MGBASE_ENABLE_LOG
    #define MGBASE_LOGGER_OUTPUT(level, ...) \
        if (::mgbase::logger::get_log_level() <= level) { \
            ::mgbase::lock_guard< ::mgbase::spinlock> lc(::mgbase::logger::get_lock()); \
            std::cout << mgbase::logger::get_state(); \
            fmt::print(__VA_ARGS__); \
            std::cout << std::endl; \
        }
#else
    #ifdef MGBASE_COMPILER_FUJITSU
        // Fujitsu compiler tries to link unused functions in if(false)
        #define MGBASE_LOGGER_OUTPUT(level, ...)
    #else
        // Do a static type check (but do nothing in run-time)
        #define MGBASE_LOGGER_OUTPUT(level, ...) \
            if (false) { mgbase::detail::logger_not_defined(__VA_ARGS__); }
    #endif
#endif

#define MGBASE_LOG_VERBOSE(...) MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define MGBASE_LOG_DEBUG(...)   MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_DEBUG  , __VA_ARGS__)
#define MGBASE_LOG_INFO(...)    MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_INFO   , __VA_ARGS__)
#define MGBASE_LOG_WARN(...)    MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_WARN   , __VA_ARGS__)
#define MGBASE_LOG_FATAL(...)   MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_FATAL  , __VA_ARGS__)

} // namespace mgbase

