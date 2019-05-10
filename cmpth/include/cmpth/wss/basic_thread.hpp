
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

struct ptr_fork_t { };

template <typename P>
class basic_thread
{
    CMPTH_DEFINE_DERIVED(P)
    
    using worker_type = typename P::worker_type;
    
    using thread_funcs_type = typename P::thread_funcs_type;
    
    using task_ref_type = typename P::task_ref_type;
    using task_pool_type = typename P::task_pool_type;
    
public:
    basic_thread() noexcept = default;
    
    basic_thread(const basic_thread&) = delete;
    
    basic_thread(basic_thread&& other) noexcept
        : tk_{}
    {
        *this = fdn::move(other);
    }
    
    template <typename Func, typename... Args>
    explicit basic_thread(Func&& func, Args&&... args)
    {
        auto& wk = worker_type::get_cur_worker();
        auto& pool = P::get_task_pool(wk);
        
        auto child_stk = pool.allocate(wk);
        
        this->tk_ =
            thread_funcs_type::fork_child_first(
                wk
            ,   fdn::move(child_stk)
            ,   invoker<fdn::decay_t<Func>, fdn::decay_t<Args>...>{
                    fdn::forward<Func>(func)
                ,   fdn::forward_as_tuple( fdn::forward<Args>(args)... )
                }
            );
    }
    
private:
    struct invoker_result {
        worker_type&    wk;
        task_pool_type& pool;
    };
    
    template <typename Func, typename... Args>
    struct invoker {
        Func                func;
        fdn::tuple<Args...> args;
        
        invoker_result operator() () {
            fdn::apply(this->func, this->args);
            
            auto& wk = worker_type::get_cur_worker();
            auto& pool = P::get_task_pool(wk);
            
            return { wk, pool };
        }
    };
    
private:
    explicit basic_thread(const task_ref_type tk)
        : tk_{tk}
    { }
    
public:
    ~basic_thread() /*noexcept*/ {
        this->destroy_this();
    }
    
private:
    void destroy_this() {
        if (CMPTH_UNLIKELY(this->tk_)) { fdn::terminate(); }
    }
    
public:
    basic_thread& operator = (const basic_thread&) = delete;
    
    basic_thread& operator = (basic_thread&& other) noexcept
    {
        destroy_this();
        
        this->tk_ = other.tk_;
        other.tk_ = {};
        
        return *this;
    }
    
    bool joinable() const noexcept {
        return !!this->tk_;
    }
    
    void join()
    {
        auto& wk = worker_type::get_cur_worker();
        auto& pool = P::get_task_pool(wk);
        
        thread_funcs_type::join(wk, pool, this->tk_);
        
        this->tk_ = task_ref_type{};
    }
    
    void detach()
    {
        auto& wk = worker_type::get_cur_worker();
        
        thread_funcs_type::detach(wk, this->tk_);
        
        this->tk_ = task_ref_type{};
    }
    
    // compatible interface with other libraries
    static derived_type ptr_fork(void (*func)(void*), void* ptr) {
        auto& wk = worker_type::get_cur_worker();
        auto& pool = P::get_task_pool(wk);
        
        auto child_stk = pool.allocate(wk);
        
        const auto tk =
            thread_funcs_type::fork_child_first(
                wk
            ,   fdn::move(child_stk)
            ,   invoke_void{ func, ptr }
            );
        
        return derived_type{tk};
    }
    
private:
    struct invoke_void {
        void (*func)(void*);
        void* ptr;
        
        invoker_result operator() () const {
            (*this->func)(this->ptr);
            
            auto& wk = worker_type::get_cur_worker();
            auto& pool = P::get_task_pool(wk);
            
            return { wk, pool };
        }
    };
    
private:
    task_ref_type tk_{};
};

} // namespace cmpth

