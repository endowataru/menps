
#pragma once

#include <mgbase/tuple.hpp>
#include <mgbase/logger.hpp>
#include <exception>

namespace mgult {

namespace detail {

template <typename F, typename... Args>
struct thread_functor
{
    // Note: This constructor is for old compilers.
    template <typename F2, typename A2>
    thread_functor(F2&& f, A2&& a)
        : func(mgbase::forward<F2>(f))
        , args(mgbase::forward<A2>(a))
    { }
    
    F                       func;
    mgbase::tuple<Args...>  args;
    
    static void invoke(void* const p)
    {
        auto& f = *static_cast<thread_functor*>(p);
        
        mgbase::apply(f.func, f.args);
        
        f.~thread_functor();
    }
};

} // namespace detail

template <typename Traits>
class basic_thread
{
    typedef typename Traits::derived_type   derived_type;
    typedef typename Traits::scheduler_type scheduler_type;
    typedef typename Traits::thread_id_type thread_id_type;
    
public:
    basic_thread() MGBASE_NOEXCEPT
        : id_(Traits::make_invalid_thread_id())
    { }
    
    basic_thread(const basic_thread&) = delete;
    
    basic_thread(basic_thread&& other) MGBASE_NOEXCEPT
        : id_(Traits::make_invalid_thread_id())
    {
        *this = mgbase::move(other);
    }
    
    template <typename F, typename... Args>
    explicit basic_thread(scheduler_type& sched, F&& f, Args&&... args)
    {
        typedef detail::thread_functor<
            typename mgbase::decay<F>::type
        ,   typename mgbase::decay<Args>::type...
        >
        functor_type;
        
        auto desc = sched.allocate(
            MGBASE_ALIGNOF(functor_type)
        ,   sizeof(functor_type)
        );
        
        new (desc.ptr) functor_type{
            mgbase::forward<F>(f)
        ,   mgbase::make_tuple(
                mgbase::forward<Args>(args)...
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
    
    basic_thread& operator = (basic_thread&& other) MGBASE_NOEXCEPT
    {
        destroy_this();
        
        // Swap the IDs.
        id_ = other.id_;
        other.id_ = Traits::make_invalid_thread_id();
        
        return *this;
    }
    
    thread_id_type get_id() const MGBASE_NOEXCEPT
    {
        return id_;
    }
    
    void join()
    {
        MGBASE_ASSERT(joinable());
        
        auto& sched = this->derived().get_scheduler();
        
        sched.join(id_);
        
        id_ = Traits::make_invalid_thread_id();
    }
    void detach()
    {
        MGBASE_ASSERT(joinable());
        
        auto& sched = this->derived().get_scheduler();
        
        sched.detach(id_);
        
        id_ = Traits::make_invalid_thread_id();
    }
    
    bool joinable() const MGBASE_NOEXCEPT
    {
        return ! Traits::is_invalid_thread_id(id_);
    }
    
private:
    void destroy_this() MGBASE_NOEXCEPT
    {
        if (joinable())
        {
            MGBASE_LOG_FATAL(
                "msg:Thread was neither joined nor detached.\t"
                "id:{:x}"
            ,   reinterpret_cast<mgbase::uintptr_t>(id_.ptr)
            );
            
            // Terminate this program.
            std::terminate();
        }
    }
    
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    
    thread_id_type id_;
};

} // namespace mgult

