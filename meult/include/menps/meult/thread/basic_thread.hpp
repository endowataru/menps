
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/tuple.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/logger.hpp>
#include <exception>

namespace menps {
namespace meult {

namespace detail {

template <typename F, typename... Args>
struct thread_functor
{
    // Note: This constructor is for old compilers.
    template <typename F2, typename A2>
    thread_functor(F2&& f, A2&& a)
        : func(mefdn::forward<F2>(f))
        , args(mefdn::forward<A2>(a))
    { }
    
    F                       func;
    mefdn::tuple<Args...>  args;
    
    static void invoke(void* const p)
    {
        auto& f = *static_cast<thread_functor*>(p);
        
        mefdn::apply(f.func, f.args);
        
        f.~thread_functor();
    }
};

} // namespace detail

template <typename Policy>
class basic_thread
{
    typedef typename Policy::derived_type   derived_type;
    typedef typename Policy::scheduler_type scheduler_type;
    typedef typename Policy::thread_id_type thread_id_type;
    
public:
    basic_thread() noexcept
        : id_(Policy::make_invalid_thread_id())
    { }
    
    basic_thread(const basic_thread&) = delete;
    
    basic_thread(basic_thread&& other) noexcept
        : id_(Policy::make_invalid_thread_id())
    {
        *this = mefdn::move(other);
    }
    
    template <typename F, typename... Args>
    explicit basic_thread(scheduler_type& sched, F&& f, Args&&... args)
    {
        typedef detail::thread_functor<
            typename mefdn::decay<F>::type
        ,   typename mefdn::decay<Args>::type...
        >
        functor_type;
        
        auto desc = sched.allocate(
            alignof(functor_type)
        ,   sizeof(functor_type)
        );
        
        new (desc.ptr) functor_type{
            mefdn::forward<F>(f)
        ,   mefdn::make_tuple(
                mefdn::forward<Args>(args)...
            )
        };
        
        id_ = desc.id;
        
        const auto invoker = &functor_type::invoke;
        
        sched.fork(desc, invoker);
    }
    
    ~basic_thread()
    {
        destroy_this();
    }
    
    basic_thread& operator = (const basic_thread&) = delete;
    
    basic_thread& operator = (basic_thread&& other) noexcept
    {
        destroy_this();
        
        // Swap the IDs.
        id_ = other.id_;
        other.id_ = Policy::make_invalid_thread_id();
        
        return *this;
    }
    
    thread_id_type get_id() const noexcept
    {
        return id_;
    }
    
    void join()
    {
        MEFDN_ASSERT(joinable());
        
        auto& sched = this->derived().get_scheduler();
        
        sched.join(id_);
        
        id_ = Policy::make_invalid_thread_id();
    }
    void detach()
    {
        MEFDN_ASSERT(joinable());
        
        auto& sched = this->derived().get_scheduler();
        
        sched.detach(id_);
        
        id_ = Policy::make_invalid_thread_id();
    }
    
    bool joinable() const noexcept
    {
        return ! Policy::is_invalid_thread_id(id_);
    }
    
private:
    void destroy_this() noexcept
    {
        if (joinable())
        {
            MEFDN_LOG_FATAL(
                "msg:Thread was neither joined nor detached.\t"
                "id:{:x}"
            ,   reinterpret_cast<mefdn::uintptr_t>(id_.ptr)
            );
            
            // Terminate this program.
            std::terminate();
        }
    }
    
    derived_type& derived() noexcept {
        return static_cast<derived_type&>(*this);
    }
    
    thread_id_type id_;
};

} // namespace meult
} // namespace menps

