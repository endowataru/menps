
#pragma once

#include <cmpth/wrap/mth/mth_itf.hpp>

#define CMPTH_AVOID_MYTH_UNCOND_SETUP
#define CMPTH_ENABLE_MTH_UNCOND_ENTER

namespace cmpth {

class mth_uncond_var
{
public:
    mth_uncond_var() /* may throw */
    {
        #ifdef CMPTH_AVOID_MYTH_UNCOND_SETUP
        this->u_.th = nullptr;
        
        #else
        myth_uncond_init(&this->u_);
        #endif
    }
    
    ~mth_uncond_var() {
        #ifndef CMPTH_AVOID_MYTH_UNCOND_SETUP
        myth_uncond_destroy(&this->u_); // ignore error
        #endif
    }
    
    mth_uncond_var(const mth_uncond_var&) = delete;
    mth_uncond_var& operator = (const mth_uncond_var&) = delete;
    
    void wait()
    {
        if (myth_uncond_wait(&this->u_) != 0)
            throw mth_error{};
    }
    
    void notify()
    {
        this->notify_signal();
    }
    
    void notify_signal()
    {
        if (myth_uncond_signal(&this->u_) != 0)
            throw mth_error{};
    }
    void notify_enter()
    {
        #ifdef CMPTH_ENABLE_MTH_UNCOND_ENTER
        if (myth_uncond_enter(&this->u_) != 0)
            throw mth_error{};
        #else
        this->notify_signal();
        #endif
    }
    
    void swap(mth_uncond_var& next_uv)
    {
        if (myth_uncond_swap(&this->u_, &next_uv.u_) != 0)
            throw mth_error{};
    }
    
private:
    template <typename Func>
    static int on_swap(void* const ptr)
    {
        auto& func = *static_cast<Func*>(ptr);
        // TODO: To avoid bugs, copy the function here.
        auto f = func;
        return f();
    }
    
public:
    template <typename Func>
    void swap_with(mth_uncond_var& next_uv, Func func)
    {
        const auto ret =
            myth_uncond_swap_with(
                &this->u_, &next_uv.u_, &on_swap<Func>, &func);
        
        if (ret != 0)
            throw mth_error{};
    }
    
private:
    template <typename Func>
    struct wait_func
    {
        Func func;
        
        static int on_wait(void* const ptr)
        {
            auto& self = *static_cast<wait_func*>(ptr);
            
            // TODO: To avoid bugs, copy the function here.
            auto f = self.func;
            return f();
        }
    };
    
public:
    template <typename Func>
    void wait_with(Func func)
    {
        wait_func<Func> f{
            fdn::move(func)
        };
        
        const auto ret =
            myth_uncond_wait_with(
                &this->u_, &wait_func<Func>::on_wait, &f);
        
        if (ret != 0)
            throw mth_error{};
    }
    
    // For interfacing in ctmth
    
    template <typename Worker>
    void wait(Worker& /*wk*/) { this->wait(); }
    
    template <typename Worker>
    void notify(Worker& /*wk*/) { this->notify(); }
    
    template <typename Worker>
    void notify_signal(Worker& /*wk*/) { this->notify_signal(); }
    
    template <typename Worker>
    void notify_enter(Worker& /*wk*/) { this->notify_enter(); }
    
    template <typename Worker>
    void swap(Worker& /*wk*/, mth_uncond_var& next_uv) {
        this->swap(next_uv);
    }
    
    template <typename Func, typename Worker, typename... Args>
    void swap_with(Worker& wk, mth_uncond_var& next_uv,
        Args* const ... args)
    {
        using func_type = call_with_ptrs<fdn::decay_t<Func>, Worker, Args...>;
        func_type f{ fdn::make_tuple(fdn::ref(wk), args...) };
        
        const auto ret =
            myth_uncond_swap_with(&this->u_, &func_type::on_wait, &f);
        
        if (ret != 0)
            throw mth_error{};
    }
    
    template <typename Func, typename Worker, typename... Args>
    void wait_with(Worker& wk, Args* const ... args)
    {
        using func_type = call_with_ptrs<fdn::decay_t<Func>, Worker, Args...>;
        func_type f{ fdn::make_tuple(fdn::ref(wk), args...) };
        
        const auto ret =
            myth_uncond_wait_with(&this->u_, &func_type::on_wait, &f);
        
        if (ret != 0)
            throw mth_error{};
    }
    
private:
    template <typename Func, typename Worker, typename... Args>
    struct call_with_ptrs
    {
        fdn::tuple<fdn::reference_wrapper<Worker>, Args*...> args;
        
        static int on_wait(void* const ptr) {
            const auto& self = *static_cast<call_with_ptrs*>(ptr);
            const bool ret = fdn::apply(Func{}, self.args);
            return ret; // Convert to int
        }
    };
    
private:
    myth_uncond_t u_;
};

} // namespace cmpth

