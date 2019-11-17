
#pragma once

#include <cmpth/fdn/fdn.hpp>

#define CMPTH_P_LOG_VERBOSE(P, ...) \
    do { \
        if (P::log_aspect_type::is_enabled) { \
            P::log_aspect_type::print_log(P::log_aspect_type::log_level_t::verbose, __VA_ARGS__); \
        } \
    } while (false)

#define CMPTH_P_LOG_DEBUG(P, ...) \
    do { \
        if (P::log_aspect_type::is_enabled) { \
            P::log_aspect_type::print_log(P::log_aspect_type::log_level_t::debug, __VA_ARGS__); \
        } \
    } while (false)

#define CMPTH_P_LOG_INFO(P, ...) \
    do { \
        if (P::log_aspect_type::is_enabled) { \
            P::log_aspect_type::print_log(P::log_aspect_type::log_level_t::info, __VA_ARGS__); \
        } \
    } while (false)

#define CMPTH_P_LOG_WARN(P, ...) \
    do { \
        if (P::log_aspect_type::is_enabled) { \
            P::log_aspect_type::print_log(P::log_aspect_type::log_level_t::warn, __VA_ARGS__); \
        } \
    } while (false)

#define CMPTH_P_LOG_FATAL(P, ...) \
    do { \
        P::log_aspect_type::print_log(P::log_aspect_type::log_level_t::fatal, __VA_ARGS__); \
    } while (false)

