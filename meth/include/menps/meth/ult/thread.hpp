
#pragma once

#include "scheduler.hpp"
#include <menps/meult/thread/basic_thread.hpp>

namespace menps {
namespace meth {
namespace ult {

class thread;

using meult::ult_id;

namespace detail {

struct thread_traits
{
    typedef thread      derived_type;
    typedef scheduler   scheduler_type;
    typedef ult_id      thread_id_type;
    
    static bool is_invalid_thread_id(const thread_id_type id) {
        return meult::is_invalid_ult_id(id);
    }
    static ult_id make_invalid_thread_id() noexcept {
        return meult::make_invalid_ult_id();
    }
};

} // namespace detail

class thread
    : public meult::basic_thread<detail::thread_traits>
{
    typedef meult::basic_thread<detail::thread_traits> base;
    
public:
    thread() noexcept = default;
    
    thread(const thread&) = delete;
    thread& operator = (const thread&) = delete;
    
    thread(thread&&) noexcept = default;
    thread& operator = (thread&&) noexcept = default;
    
    template <typename F, typename... Args>
    explicit thread(F&& f, Args&&... args)
        : base{
            meth::ult::get_scheduler()
        ,   mefdn::forward<F>(f)
        ,   mefdn::forward<Args>(args)...
        }
    { }
    
    scheduler& get_scheduler() { return meth::ult::get_scheduler(); }
};

} // namespace ult
} // namespace meth
} // namespace menps

