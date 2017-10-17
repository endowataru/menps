
#pragma once

#include <functional>
#include <iostream>

#include <mgbase/external/fmt.hpp>
//#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/recursive_spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>

namespace mgbase {

#define MGBASE_LOG_LEVEL_VERBOSE    0
#define MGBASE_LOG_LEVEL_DEBUG      1
#define MGBASE_LOG_LEVEL_INFO       10
#define MGBASE_LOG_LEVEL_WARN       100
#define MGBASE_LOG_LEVEL_FATAL      1000

class logger {
private:
    typedef recursive_spinlock  lock_type;
    
public:
    typedef mgbase::uint32_t log_level_t;
    
    typedef std::function<std::string ()>   state_callback_type;
    
    template <typename... Args>
    static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const Args&...      args)
    {
        if (level < get_log_level()) {
            return;
        }
        
        fmt::MemoryWriter out;
        
        out << logger::get_state();
        
        out.write(fmt_str, args...);
        
        const auto s = out.str();
        
        mgbase::lock_guard<lock_type> lc(get_lock());
        std::cout << s << std::endl;
    }
    
public:
    static void set_state_callback(state_callback_type callback)
    {
        mgbase::lock_guard<lock_type> lc(get_state_lock());
        
        get_state_callback() = callback;
    }
    
private:
    static std::string get_state() {
        mgbase::lock_guard<lock_type> lc(get_state_lock());
        
        const state_callback_type& callback = get_state_callback();
        if (callback)
            return callback();
        else
            return std::string();
    }
    
    static log_level_t get_log_level() {
        static const log_level_t result = get_log_level_from_env();
        return result;
    }
    
    // TODO: Singleton
    static lock_type& get_lock() {
        static lock_type lc;
        return lc;
    }
    
    // TODO: Singleton
    static lock_type& get_state_lock() {
        static lock_type lc;
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
        static state_callback_type callback{};
        return callback;
    }
};

#ifndef MGBASE_ENABLE_LOG
    #ifdef MGBASE_DEBUG
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
        ::mgbase::logger::add_log(level, __VA_ARGS__)
#else
    #define MGBASE_LOGGER_OUTPUT(level, ...)
#endif

#define MGBASE_LOG_VERBOSE(...) MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define MGBASE_LOG_DEBUG(...)   MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_DEBUG  , __VA_ARGS__)
#define MGBASE_LOG_INFO(...)    MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_INFO   , __VA_ARGS__)
#define MGBASE_LOG_WARN(...)    MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_WARN   , __VA_ARGS__)
#define MGBASE_LOG_FATAL(...)   MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_FATAL  , __VA_ARGS__)

} // namespace mgbase

