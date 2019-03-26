
#pragma once

#include <iostream> // TODO

namespace cmpth {

template <bool IsEnabled>
struct log_policy
{
    static const bool is_enabled = IsEnabled;
};

} // namespace cmpth

#define CMPTH_P_LOG_ARGS_0()
#define CMPTH_P_LOG_ARGS_1(name, val, ...)  << name << ":" << val << "\t" CMPTH_P_LOG_ARGS_0(__VA_ARGS__)
#define CMPTH_P_LOG_ARGS_2(name, val, ...)  << name << ":" << val << "\t" CMPTH_P_LOG_ARGS_1(__VA_ARGS__)
#define CMPTH_P_LOG_ARGS_3(name, val, ...)  << name << ":" << val << "\t" CMPTH_P_LOG_ARGS_2(__VA_ARGS__)

#define CMPTH_P_LOG_DEBUG(P, msg, n, ...) \
    do { \
        if (P::log_policy_type::is_enabled) { \
        /*if (P::get_log_level() >= 1) {*/ /*TODO*/ \
            std::cerr << "msg:" << msg << "\t" \
                CMPTH_P_LOG_ARGS_##n(__VA_ARGS__) \
                << std::endl; \
        } \
    } while (false) \

#define CMPTH_P_LOG_FATAL(P, msg, n, ...) \
    do { \
        std::cerr << "msg:" << msg << "\t" \
            CMPTH_P_LOG_ARGS_##n(__VA_ARGS__) \
            << std::endl; \
    } while (false) \

