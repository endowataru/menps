
#pragma once

#include <cmpth/fdn/fdn.hpp>
#include <iostream> // TODO
#include <sstream>

namespace cmpth {

using log_filter_flag_t = fdn::size_t;

template <bool IsEnabled>
struct log_aspect_base
{
    static const bool is_enabled = IsEnabled;

    enum class log_level_t {
        verbose = 1
    ,   debug   = 2
    ,   info    = 3
    ,   warn    = 4
    ,   fatal   = 5
    };
    
    static log_level_t get_log_level() {
        static const char* const log_level_str = std::getenv("CMPTH_LOG_LEVEL");
        static const auto ret =
                log_level_str
            ?   static_cast<log_level_t>(std::atoi(log_level_str))
            :   log_level_t::warn;
        
        return ret;
    }

    static log_filter_flag_t get_filter_flag() {
        static const char* const filter_str = std::getenv("CMPTH_LOG_FILTER");
        static const auto ret =
                filter_str
            ?   static_cast<log_filter_flag_t>(std::atoi(filter_str))
            :   ~static_cast<log_filter_flag_t>(0);
        
        return ret;
    }
    
    using state_callback_type = std::function<std::string ()>;
    static state_callback_type& get_state_callback() {
        static state_callback_type cb =
            [] {
                std::ostringstream os;
                os << "thread:";
                os << std::hex << fdn::force_integer_cast<fdn::intptr_t>(pthread_self());
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
    static void print_log(
        const log_filter_flag_t flag
    ,   const log_level_t       level
    ,   const char* const       msg
    ,   Rest&&...               rest
    ) {
        if ((flag & get_filter_flag()) == 0) { return; }
        if (level < get_log_level()) { return; }

        std::ostringstream os;
        os << get_state();
        os << "\tflag:" << static_cast<fdn::size_t>(flag);
        os << "\tlevel:" << static_cast<int>(level);
        os << "\tmsg:" << msg;
        print_var_list(os, fdn::forward<Rest>(rest)...);
        os << std::endl;
        
        const auto str = os.str();
        
        if (level == log_level_t::fatal) {
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
        os << "0x" << std::hex << reinterpret_cast<fdn::intptr_t>(ptr) << std::dec;
    }
};

template <bool IsEnabled, log_filter_flag_t Flag>
struct log_aspect
    : log_aspect_base<IsEnabled>
{
    using typename log_aspect_base<IsEnabled>::log_level_t;

    template <log_filter_flag_t Flag2>
    using rebind = log_aspect<IsEnabled, Flag2>;

    template <typename... Rest>
    static void print_log(
        const log_level_t       level
    ,   const char* const       msg
    ,   Rest&&...               rest
    ) {
        log_aspect_base<IsEnabled>::print_log(
            Flag, level, msg, fdn::forward<Rest>(rest)...);
    }
};

using def_log_aspect =
    log_aspect<
        #ifdef CMPTH_DEBUG
        true
        #else
        false
        #endif
    ,   1
    >;

} // namespace cmpth

