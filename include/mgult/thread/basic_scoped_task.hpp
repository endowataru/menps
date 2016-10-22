
#pragma once

#include <mgbase/utility/forward.hpp>
#include <mgbase/tuple/apply.hpp>

namespace mgult {

namespace detail {

template <typename T>
struct scoped_task_functor
{
    T* result;
    
    template <typename Func, typename... Args>
    void operator() (Func&& func, Args&&... args)
    {
        *result = mgbase::forward<Func>(func)(mgbase::forward<Args>(args)...);
    }
};

} // namespace detail

template <typename Traits, typename T>
class basic_scoped_task
{
    typedef typename Traits::derived_type   derived_type;
    typedef typename Traits::thread_type    thread_type;
    
public:
    template <typename Func, typename... Args>
    explicit basic_scoped_task(Func&& func, Args&&... args)
        : th_(
            detail::scoped_task_functor<T>{ &result_ }
        ,   mgbase::forward<Func>(func)
        ,   mgbase::forward<Args>(args)...
        )
    { }
    
    basic_scoped_task(const basic_scoped_task&) = delete;
    
    basic_scoped_task& operator = (const basic_scoped_task&) = delete;
    
    T& get()
    {
        wait();
        return result_;
    }
    
    void wait()
    {
        if (th_.joinable()) {
            th_.join();
        }
    }
    
private:
    thread_type th_;
    T           result_;
};

} // namespace mgult

