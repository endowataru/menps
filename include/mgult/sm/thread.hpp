
#pragma once

#include "scheduler.hpp"
#include <mgult/thread/basic_thread.hpp>

namespace mgult {
namespace sm {

class thread;

namespace detail {

struct thread_traits
{
    typedef thread      derived_type;
    typedef scheduler   scheduler_type;
    typedef ult_id      thread_id_type;
    
    static bool is_invalid_thread_id(const thread_id_type id) MGBASE_NOEXCEPT {
        return is_invalid_ult_id(id);
    }
    static ult_id make_invalid_thread_id() MGBASE_NOEXCEPT {
        return make_invalid_ult_id();
    }
};

} // namespace detail

class thread
    : public basic_thread<detail::thread_traits>
{
    typedef basic_thread<detail::thread_traits> base;
    
public:
    thread() MGBASE_DEFAULT_NOEXCEPT = default;
    
    thread(const thread&) = delete;
    
    thread(thread&&) MGBASE_NOEXCEPT = default;
    
    template <typename F, typename... Args>
    explicit thread(F&& f, Args&&... args)
        : base{ sm::get_scheduler(), mgbase::forward<F>(f), mgbase::forward<Args>(args)... }
    { }
    
    thread& operator = (const thread&) = delete;
    
    thread& operator = (thread&&) MGBASE_NOEXCEPT = default;
    
    scheduler& get_scheduler() { return sm::get_scheduler(); }
};

} // namespace sm
} // namespace mgult

