
#pragma once

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
    typedef mgbase::uint32_t log_level_t;
    
    typedef std::string (*state_callback_type)();
    
    static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ) {
        if (!start_print_log(level)) return;
        fmt::print(fmt_str);
        finish_print_log();
    }
    template <typename A1>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    );
    template <typename A1, typename A2>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    ,   const A2&           a2
    );
    template <typename A1, typename A2, typename A3>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    ,   const A2&           a2
    ,   const A3&           a3
    );
    template <typename A1, typename A2, typename A3, typename A4>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    ,   const A2&           a2
    ,   const A3&           a3
    ,   const A4&           a4
    );
    template <typename A1, typename A2, typename A3, typename A4, typename A5>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    ,   const A2&           a2
    ,   const A3&           a3
    ,   const A4&           a4
    ,   const A5&           a5
    );
    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    ,   const A2&           a2
    ,   const A3&           a3
    ,   const A4&           a4
    ,   const A5&           a5
    ,   const A6&           a6
    );
    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    ,   const A2&           a2
    ,   const A3&           a3
    ,   const A4&           a4
    ,   const A5&           a5
    ,   const A6&           a6
    ,   const A7&           a7
    );
    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    ,   const A2&           a2
    ,   const A3&           a3
    ,   const A4&           a4
    ,   const A5&           a5
    ,   const A6&           a6
    ,   const A7&           a7
    ,   const A8&           a8
    );
    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    ,   const A2&           a2
    ,   const A3&           a3
    ,   const A4&           a4
    ,   const A5&           a5
    ,   const A6&           a6
    ,   const A7&           a7
    ,   const A8&           a8
    ,   const A9&           a9
    );
    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
    MGBASE_NOINLINE static void add_log(
        const log_level_t   level
    ,   const char* const   fmt_str
    ,   const A1&           a1
    ,   const A2&           a2
    ,   const A3&           a3
    ,   const A4&           a4
    ,   const A5&           a5
    ,   const A6&           a6
    ,   const A7&           a7
    ,   const A8&           a8
    ,   const A9&           a9
    ,   const A10&          a10
    );

private:
    static bool start_print_log(const log_level_t level)
    {
        if (level < get_log_level())
            return false;
        
        get_lock().lock();
        std::cout << mgbase::logger::get_state();
        return true;
    }
    static void finish_print_log()
    {
        std::cout << std::endl;
        get_lock().unlock();
    }

public:
    static void set_state_callback(state_callback_type callback) {
        get_state_callback() = callback;
    }
    
private:
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

template <typename A1>
MGBASE_NOINLINE void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1);
    finish_print_log();
}
template <typename A1, typename A2>
MGBASE_NOINLINE void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
,   const A2&           a2
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1, a2);
    finish_print_log();
}
template <typename A1, typename A2, typename A3>
MGBASE_NOINLINE void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
,   const A2&           a2
,   const A3&           a3
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1, a2, a3);
    finish_print_log();
}
template <typename A1, typename A2, typename A3, typename A4>
MGBASE_NOINLINE void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
,   const A2&           a2
,   const A3&           a3
,   const A4&           a4
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1, a2, a3, a4);
    finish_print_log();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5>
MGBASE_NOINLINE void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
,   const A2&           a2
,   const A3&           a3
,   const A4&           a4
,   const A5&           a5
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1, a2, a3, a4, a5);
    finish_print_log();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
MGBASE_NOINLINE void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
,   const A2&           a2
,   const A3&           a3
,   const A4&           a4
,   const A5&           a5
,   const A6&           a6
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1, a2, a3, a4, a5, a6);
    finish_print_log();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
MGBASE_NOINLINE  void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
,   const A2&           a2
,   const A3&           a3
,   const A4&           a4
,   const A5&           a5
,   const A6&           a6
,   const A7&           a7
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1, a2, a3, a4, a5, a6, a7);
    finish_print_log();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
MGBASE_NOINLINE  void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
,   const A2&           a2
,   const A3&           a3
,   const A4&           a4
,   const A5&           a5
,   const A6&           a6
,   const A7&           a7
,   const A8&           a8
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1, a2, a3, a4, a5, a6, a7, a8);
    finish_print_log();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
MGBASE_NOINLINE  void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
,   const A2&           a2
,   const A3&           a3
,   const A4&           a4
,   const A5&           a5
,   const A6&           a6
,   const A7&           a7
,   const A8&           a8
,   const A9&           a9
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    finish_print_log();
}
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
MGBASE_NOINLINE  void logger::add_log(
    const log_level_t   level
,   const char* const   fmt_str
,   const A1&           a1
,   const A2&           a2
,   const A3&           a3
,   const A4&           a4
,   const A5&           a5
,   const A6&           a6
,   const A7&           a7
,   const A8&           a8
,   const A9&           a9
,   const A10&          a10
) {
    if (!start_print_log(level)) return;
    fmt::print(fmt_str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    finish_print_log();
}

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
    /*#define MGBASE_LOGGER_OUTPUT(level, ...) \
        if (::mgbase::logger::get_log_level() <= level) { \
            ::mgbase::lock_guard< ::mgbase::spinlock> logger_lc(::mgbase::logger::get_lock()); \
            std::cout << mgbase::logger::get_state(); \
            fmt::print(__VA_ARGS__); \
            std::cout << std::endl; \
        }*/
#else
    //#if defined(MGBASE_COMPILER_FUJITSU) | defined(MGBASE_COMPILER_CLANG) 
        // Fujitsu compiler tries to link unused functions in if(false)
        // Clang doesn't permit passing non-POD types to variadic function
        #define MGBASE_LOGGER_OUTPUT(level, ...)
    /*#else
        // Do a static type check (but do nothing in run-time)
        #define MGBASE_LOGGER_OUTPUT(level, ...) \
            if (false) { mgbase::detail::logger_not_defined(__VA_ARGS__); }
    #endif*/
#endif

#define MGBASE_LOG_VERBOSE(...) MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define MGBASE_LOG_DEBUG(...)   MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_DEBUG  , __VA_ARGS__)
#define MGBASE_LOG_INFO(...)    MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_INFO   , __VA_ARGS__)
#define MGBASE_LOG_WARN(...)    MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_WARN   , __VA_ARGS__)
#define MGBASE_LOG_FATAL(...)   MGBASE_LOGGER_OUTPUT(MGBASE_LOG_LEVEL_FATAL  , __VA_ARGS__)

} // namespace mgbase

