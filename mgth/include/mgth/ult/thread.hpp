
#pragma once

#include "scheduler.hpp"
#include <mgult/thread/basic_thread.hpp>

namespace mgth {
namespace ult {

class thread;

using mgult::ult_id;

namespace detail {

struct thread_traits
{
    typedef thread      derived_type;
    typedef scheduler   scheduler_type;
    typedef ult_id      thread_id_type;
    
    static bool is_invalid_thread_id(const thread_id_type id) {
        return mgult::is_invalid_ult_id(id);
    }
    static ult_id make_invalid_thread_id() MGBASE_NOEXCEPT {
        return mgult::make_invalid_ult_id();
    }
};

} // namespace detail

class thread
    : public mgult::basic_thread<detail::thread_traits>
{
    typedef mgult::basic_thread<detail::thread_traits> base;
    
public:
    thread() MGBASE_DEFAULT_NOEXCEPT = default;
    
    thread(const thread&) = delete;
    thread& operator = (const thread&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(thread, base)
    
    template <typename F, typename... Args>
    explicit thread(F&& f, Args&&... args)
        : base{
            mgth::ult::get_scheduler()
        ,   mgbase::forward<F>(f)
        ,   mgbase::forward<Args>(args)...
        }
    { }
    
    scheduler& get_scheduler() { return mgth::ult::get_scheduler(); }
};

} // namespace ult
} // namespace mgth

