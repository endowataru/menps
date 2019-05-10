
#pragma once

#include <cmpth/fdn/fdn.hpp>
#include <iostream> // TODO
#include <sstream>

namespace cmpth {

template <bool IsEnabled>
struct log_policy
{
    static const bool is_enabled = IsEnabled;
    
    static int get_log_level() {
        static const char* const log_level_str = std::getenv("CMPTH_LOG_LEVEL");
        static const int log_level = log_level_str ? std::atoi(log_level_str) : 100;
        return log_level;
    }
    static bool is_debug_level() {
        return get_log_level() <= 1;
    }
    
    using state_callback_type = std::function<std::string ()>;
    static state_callback_type& get_state_callback() {
        static state_callback_type cb =
            [] {
                std::ostringstream os;
                os << "thread:";
                os << std::hex << static_cast<fdn::intptr_t>(pthread_self());
                return os.str();
            };
        return cb;
    }
    static void set_state_callback(state_callback_type cb) {
        get_state_callback() = cb;
    }
    static std::string get_state() {
        return get_state_callback()();
    }
};

#if 0
template <typename T>
inline fdn::string make_log_val_str(const char* const name, const T& val) {
    std::ostringstream os;
    os << name << ":" << val;
    return os.str();
}
template <typename T>
inline fdn::string make_log_val_str(const char* const name, const T& val) {
    std::ostringstream os;
    os << name << ":" << val;
    return os.str();
}
#endif

} // namespace cmpth

#define CMPTH_P_LOG_ARGS_0()
#define CMPTH_P_LOG_ARGS_1(name, val, ...)  << name << ":" << val << "\t" CMPTH_P_LOG_ARGS_0(__VA_ARGS__)
#define CMPTH_P_LOG_ARGS_2(name, val, ...)  << name << ":" << val << "\t" CMPTH_P_LOG_ARGS_1(__VA_ARGS__)
#define CMPTH_P_LOG_ARGS_3(name, val, ...)  << name << ":" << val << "\t" CMPTH_P_LOG_ARGS_2(__VA_ARGS__)

#define CMPTH_P_LOG_DEBUG(P, msg, n, ...) \
    do { \
        if (P::log_policy_type::is_enabled) { \
            if (P::log_policy_type::is_debug_level()) { \
                std::cerr << \
                    P::log_policy_type::get_state() << "\t" << \
                    "msg:" << msg << "\t" \
                    CMPTH_P_LOG_ARGS_##n(__VA_ARGS__) \
                    << std::endl; \
            } \
        } \
    } while (false) \

#define CMPTH_P_LOG_FATAL(P, msg, n, ...) \
    do { \
        std::cerr << \
            P::log_policy_type::get_state() << "\t" << \
            "msg:" << msg << "\t" \
            CMPTH_P_LOG_ARGS_##n(__VA_ARGS__) \
            << std::endl; \
    } while (false) \

