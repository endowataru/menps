
#pragma once

#include <menps/mefdn/external/fmt.hpp>
#include <menps/mefdn/thread/recursive_spinlock.hpp>
#include <menps/mefdn/thread/lock_guard.hpp>

#include <functional>
#include <iostream>
#include <string>

namespace menps {
namespace mefdn {

#define MEFDN_LOG_LEVEL_VERBOSE    0
#define MEFDN_LOG_LEVEL_DEBUG      1
#define MEFDN_LOG_LEVEL_INFO       10
#define MEFDN_LOG_LEVEL_WARN       100
#define MEFDN_LOG_LEVEL_FATAL      1000

class logger {
private:
    typedef recursive_spinlock  lock_type;
    
public:
    typedef mefdn::uint_least16_t log_level_t;
    
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
        
        mefdn::lock_guard<lock_type> lc(get_lock());
        std::cout << s << std::endl;
    }
    
public:
    static void set_state_callback(state_callback_type callback)
    {
        mefdn::lock_guard<lock_type> lc(get_state_lock());
        
        get_state_callback() = callback;
    }
    
private:
    static std::string get_state() {
        mefdn::lock_guard<lock_type> lc(get_state_lock());
        
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
        const char* const value = std::getenv("MEFDN_LOG_LEVEL");
        if (value == nullptr)
            return MEFDN_LOG_LEVEL_FATAL;
        
        return static_cast<log_level_t>(std::atoi(value));
    }
    
    // TODO: Singleton
    static state_callback_type& get_state_callback() {
        static state_callback_type callback{};
        return callback;
    }
};

#ifndef MEFDN_ENABLE_LOG
    #ifdef MEFDN_DEBUG
        #define MEFDN_ENABLE_LOG
    #else
        //#define MEFDN_ENABLE_LOG // Disabled
    #endif
#endif

namespace detail {

// This function is never defined.
void logger_not_defined(...);

} // namespace detail

#define MEFDN_LOGGER_OUTPUT_ALWAYS(level, ...) \
    ::menps::mefdn::logger::add_log(level, __VA_ARGS__)

#ifdef MEFDN_ENABLE_LOG
    #define MEFDN_LOGGER_OUTPUT(level, ...) \
        MEFDN_LOGGER_OUTPUT_ALWAYS(level, __VA_ARGS__)
#else
    #define MEFDN_LOGGER_OUTPUT(level, ...)
#endif

#define MEFDN_LOG_VERBOSE(...) MEFDN_LOGGER_OUTPUT(MEFDN_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define MEFDN_LOG_DEBUG(...)   MEFDN_LOGGER_OUTPUT(MEFDN_LOG_LEVEL_DEBUG  , __VA_ARGS__)
#define MEFDN_LOG_INFO(...)    MEFDN_LOGGER_OUTPUT(MEFDN_LOG_LEVEL_INFO   , __VA_ARGS__)
#define MEFDN_LOG_WARN(...)    MEFDN_LOGGER_OUTPUT(MEFDN_LOG_LEVEL_WARN   , __VA_ARGS__)
#define MEFDN_LOG_FATAL(...)   MEFDN_LOGGER_OUTPUT_ALWAYS(MEFDN_LOG_LEVEL_FATAL, __VA_ARGS__)


template <typename T>
inline T show_param(const T& val) {
    return val;
}
template <typename T>
inline std::string show_param(T* const val) {
    return fmt::format("0x{:x}", reinterpret_cast<mefdn::uintptr_t>(val));
}

} // namespace mefdn
} // namespace menps

