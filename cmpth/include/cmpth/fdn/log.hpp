
#pragma once

#include <cmpth/fdn/fdn.hpp>
#include <iostream> // TODO
#include <sstream>

namespace cmpth {

template <bool IsEnabled>
struct log_policy
{
    static const bool is_enabled = IsEnabled;
    
    enum class log_level {
        verbose = 0
    ,   debug   = 1
    ,   info    = 10
    ,   warn    = 100
    ,   fatal   = 1000
    };
    
    static log_level get_log_level() {
        static const char* const log_level_str = std::getenv("CMPTH_LOG_LEVEL");
        static const int log_level_int = log_level_str ? std::atoi(log_level_str) : 100;
        
        return static_cast<log_level>(log_level_int);
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
    
    template <typename... Rest>
    static void print_verbose_log(Rest&&... rest) {
        print_log(log_level::verbose, fdn::forward<Rest>(rest)...);
    }
    template <typename... Rest>
    static void print_debug_log(Rest&&... rest) {
        print_log(log_level::debug, fdn::forward<Rest>(rest)...);
    }
    template <typename... Rest>
    static void print_info_log(Rest&&... rest) {
        print_log(log_level::info, fdn::forward<Rest>(rest)...);
    }
    template <typename... Rest>
    static void print_warn_log(Rest&&... rest) {
        print_log(log_level::warn, fdn::forward<Rest>(rest)...);
    }
    template <typename... Rest>
    static void print_fatal_log(Rest&&... rest) {
        print_log(log_level::fatal, fdn::forward<Rest>(rest)...);
    }
    
    template <typename... Rest>
    static void print_log(const log_level level, const char* const msg, int /*num_params*/, Rest&&... rest)
    {
        if (level < get_log_level()) { return; }
        
        std::ostringstream os;
        os << get_state();
        os << "\tlevel:" << static_cast<int>(level);
        os << "\tmsg:" << msg;
        print_var_list(os, fdn::forward<Rest>(rest)...);
        os << std::endl;
        
        const auto str = os.str();
        
        if (level == log_level::fatal) {
            std::cerr << str;
            std::cout << str;
        }
        else {
            std::cout << str;
        }
    }
    
private:
    static void print_var_list(std::ostringstream& /*os*/) { }
    
    template <typename T, typename... Rest>
    static void print_var_list(std::ostringstream& os, const char* const name, T&& var, Rest&&... rest) {
        os << "\t" << name << ":";
        print_var(os, fdn::forward<T>(var));
        print_var_list(os, fdn::forward<Rest>(rest)...);
    }
    
    template <typename T>
    static void print_var(std::ostringstream& os, const T& var) {
        os << var;
    }
    template <typename T>
    static void print_var(std::ostringstream& os, T* const ptr) {
        os << "0x" << reinterpret_cast<fdn::intptr_t>(ptr);
    }
};

} // namespace cmpth

#define CMPTH_P_LOG_VERBOSE(P, ...) \
    do { \
        if (P::log_policy_type::is_enabled) { \
            P::log_policy_type::print_verbose_log(__VA_ARGS__); \
        } \
    } while (false)

#define CMPTH_P_LOG_DEBUG(P, ...) \
    do { \
        if (P::log_policy_type::is_enabled) { \
            P::log_policy_type::print_debug_log(__VA_ARGS__); \
        } \
    } while (false)

#define CMPTH_P_LOG_INFO(P, ...) \
    do { \
        if (P::log_policy_type::is_enabled) { \
            P::log_policy_type::print_info_log(__VA_ARGS__); \
        } \
    } while (false)

#define CMPTH_P_LOG_WARN(P, ...) \
    do { \
        if (P::log_policy_type::is_enabled) { \
            P::log_policy_type::print_warn_log(__VA_ARGS__); \
        } \
    } while (false)

#define CMPTH_P_LOG_FATAL(P, ...) \
    do { \
        P::log_policy_type::print_fatal_log(__VA_ARGS__); \
    } while (false)


