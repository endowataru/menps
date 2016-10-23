
#pragma once

#include "thread.hpp"
#include <mgult/thread/basic_scoped_task.hpp>

namespace mgth {
namespace ult {

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
    : public mgult::basic_scoped_task<detail::scoped_task_traits<T>, T>
{
    typedef mgult::basic_scoped_task<detail::scoped_task_traits<T>, T> base;
    
public:
    template <typename Func, typename... Args>
    scoped_task(Func&& func, Args&&... args)
        : base(mgbase::forward<Func>(func), mgbase::forward<Args>(args)...) { }
    
    scoped_task(const scoped_task&) = delete;
    
    scoped_task& operator = (const scoped_task&) = delete;
};

} // namespace ult
} // namespace mgult

