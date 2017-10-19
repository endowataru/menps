
#pragma once

#include "thread.hpp"
#include <menps/meult/thread/basic_scoped_task.hpp>

namespace menps {
namespace meult {
namespace sm {

template <typename T>
class scoped_task;

namespace detail {

template <typename T>
struct scoped_task_traits
{
    typedef scoped_task<T>  derived_type;
    typedef thread          thread_type;
};

} // namespace detail

template <typename T>
class scoped_task
    : public basic_scoped_task<detail::scoped_task_traits<T>, T>
{
    typedef basic_scoped_task<detail::scoped_task_traits<T>, T> base;
    
public:
    template <typename Func, typename... Args>
    scoped_task(Func&& func, Args&&... args)
        : base(mefdn::forward<Func>(func), mefdn::forward<Args>(args)...) { }
    
    scoped_task(const scoped_task&) = delete;
    
    scoped_task& operator = (const scoped_task&) = delete;
};

} // namespace sm
} // namespace meult
} // namespace menps

