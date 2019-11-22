
#pragma once

#include <cmpth/wrap/mth/mth.hpp>

namespace cmpth {

template <typename P>
class mth_suspended_thread
{
    using worker_type = typename P::worker_type;
    using constants_type = typename P::constants_type;
    
public:
    mth_suspended_thread() noexcept
        : u_()
    {
        if (constants_type::call_myth_uncond_setup) {
            myth_uncond_init(&this->u_);
        }
    }
    
    ~mth_suspended_thread() {
        if (constants_type::call_myth_uncond_setup) {
            myth_uncond_destroy(&this->u_); // ignore error
        }
    }
    
    mth_suspended_thread(const mth_suspended_thread&) noexcept = delete;
    mth_suspended_thread& operator = (const mth_suspended_thread&) noexcept = delete;
    
    mth_suspended_thread(mth_suspended_thread&& other) noexcept
        : u_()
    {
        *this = fdn::move(other);
    }

    mth_suspended_thread& operator = (mth_suspended_thread&& other) noexcept {
        this->u_ = other.u_;
        other.u_ = myth_uncond_t();
        return *this;
    }
    
    template <typename Func, typename... Args>
    void wait_with(Args* const ... args)
    {
        auto& wk = worker_type::get_cur_worker();
        this->template wait_with<Func>(wk, args...);
    }
    
    template <typename Func, typename... Args>
    void wait_with(worker_type& wk, Args* const ... args)
    {
        using func_type = on_wait_with<Func, Args...>;
        func_type func{ fdn::make_tuple(fdn::ref(wk), args...) };
        
        mth_error::check_error(
            myth_uncond_wait_with(&this->u_, &func_type::f, &func)
        );
    }
    
private:
    template <typename Func, typename... Args>
    struct on_wait_with
    {
        fdn::tuple<fdn::reference_wrapper<worker_type>, Args*...> args;
        
        static int f(void* const self_void) noexcept {
            // Copy the arguments from the previous call stack.
            auto self = *static_cast<on_wait_with*>(self_void);
            try {
                return fdn::apply(Func(), self.args);
            }
            catch (...) {
                CMPTH_P_LOG_FATAL(P, "An exception is thrown in myth_uncond_wait_with()!");
                std::abort();
            }
        }
    };
    
public:
    void notify()
    {
        mth_error::check_error(
            myth_uncond_signal(&this->u_)
        );
    }
    void notify(worker_type& /*wk*/)
    {
        this->notify();
    }
    
    void enter()
    {
        mth_error::check_error(
            myth_uncond_enter(&this->u_)
        );
    }
    void enter(worker_type& /*wk*/)
    {
        this->enter();
    }
    
    void swap(mth_suspended_thread& restored_sth)
    {
        mth_error::check_error(
            myth_uncond_swap(&this->u_, &restored_sth.u_)
        );
    }
    void swap(worker_type& /*wk*/, mth_suspended_thread& restored_sth)
    {
        this->swap(restored_sth);
    }
    
    template <typename Func, typename... Args>
    void swap_with(mth_suspended_thread& restored_sth, Args* const ... args)
    {
        auto& wk = worker_type::get_cur_worker();
        this->template swap_with<Func>(wk, restored_sth, args...);
    }
    template <typename Func, typename... Args>
    void swap_with(worker_type& wk, mth_suspended_thread& restored_sth, Args* const ... args)
    {
        using func_type = on_swap_with<Func, Args...>;
        func_type func{ fdn::make_tuple(fdn::ref(wk), args...) };
        
        mth_error::check_error(
            myth_uncond_swap_with(&this->u_, &restored_sth.u_,
                &func_type::f, &func)
        );
    }
    
private:
    template <typename Func, typename... Args>
    struct on_swap_with
    {
        fdn::tuple<fdn::reference_wrapper<worker_type>, Args*...> args;
        
        static int f(void* const self_void) noexcept {
            // Copy the arguments from the previous call stack.
            auto self = *static_cast<on_swap_with*>(self_void);
            try {
                return fdn::apply(Func(), self.args);
            }
            catch (...) {
                CMPTH_P_LOG_FATAL(P, "An exception is thrown in myth_uncond_swap_with()!");
                std::abort();
            }
        }
    };
    
public:
    explicit operator bool() const noexcept {
        return this->u_.th != nullptr;
    }
    
private:
    myth_uncond_t u_;
};

} // namespace cmpth

