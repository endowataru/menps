
#pragma once

#include <menps/meult/sm/scheduler.hpp>
#include <menps/meult/thread/basic_thread.hpp>

namespace menps {
namespace meult {
namespace sm {

class thread;

namespace detail {

struct thread_traits
{
    typedef thread          derived_type;
    typedef sm_scheduler    scheduler_type;
    typedef ult_id          thread_id_type;
    
    static bool is_invalid_thread_id(const thread_id_type id) noexcept {
        return is_invalid_ult_id(id);
    }
    static ult_id make_invalid_thread_id() noexcept {
        return make_invalid_ult_id();
    }
};

} // namespace detail

class thread
    : public basic_thread<detail::thread_traits>
{
    typedef basic_thread<detail::thread_traits> base;
    
public:
    thread() noexcept = default;
    
    template <typename F, typename... Args>
    explicit thread(F&& f, Args&&... args)
        : base{
            sm::get_scheduler()
        ,   mefdn::forward<F>(f)
        ,   mefdn::forward<Args>(args)...
        }
    { }
    
    thread(const thread&) = delete;
    thread& operator = (const thread&) = delete;
    
    thread(thread&&) noexcept = default;
    thread& operator = (thread&&) noexcept = default;
    
    static sm_scheduler& get_scheduler() {
        return sm::get_scheduler();
    }
};

} // namespace sm
} // namespace meult
} // namespace menps

