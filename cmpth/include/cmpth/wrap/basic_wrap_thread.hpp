
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_wrap_thread
{
    CMPTH_DEFINE_DERIVED(P)
    
    using thread_ptr_type = typename P::thread_ptr_type;
    
public:
    basic_wrap_thread() noexcept = default;
    
    template <typename F, typename... Args>
    explicit basic_wrap_thread(F&& f, Args&&... args)
    {
        using starter_type =
            starter<fdn::decay_t<F>, fdn::decay_t<Args>...>;
        
        const auto fp = &starter_type::start;
        const auto p =
            new starter_type{
                fdn::forward<F>(f)
            ,   fdn::make_tuple( fdn::forward<Args>(args)... )
            };
        
        this->t_ = P::thread_create(fp, p);
    }
    
private:
    template <typename F, typename... Args>
    struct starter
    {
        F                   func;
        fdn::tuple<Args...> args;
        
        static void* start(void* p)
        {
            auto* const self = static_cast<starter*>(p);
            try {
                fdn::apply(self->func, self->args);
                delete self;
            }
            catch (std::exception& e) {
                CMPTH_P_LOG_FATAL(P,
                    "An exception thrown in thread::thread()", 1,
                    "what", e.what()
                );
                fdn::terminate();
            }
            catch (...) {
                CMPTH_P_LOG_FATAL(P,
                    "Unknown exception thrown in thread::thread()", 0
                );
                fdn::terminate();
            }
            return nullptr;
        }
    };
    
    explicit basic_wrap_thread(const thread_ptr_type t) noexcept
        : t_{t}
    { }
    
public:
    ~basic_wrap_thread() /*noexcept*/ {
        this->destroy_this();
    }
    
private:
    void destroy_this() noexcept {
        if (CMPTH_UNLIKELY(this->t_)) { fdn::terminate(); }
    }
    
public:
    basic_wrap_thread(const basic_wrap_thread&) = delete;
    basic_wrap_thread& operator = (const basic_wrap_thread&) = delete;
    
    basic_wrap_thread(basic_wrap_thread&& other) noexcept {
        *this = fdn::move(other);
    }
    
    derived_type& operator = (basic_wrap_thread&& other) noexcept
    {
        destroy_this();
        
        this->t_ = other.t_;
        other.t_ = nullptr;
        
        return this->derived();
    }
    
    bool joinable() const noexcept {
        return this->t_ != nullptr;
    }
    
    void join()
    {
        P::thread_join(this->t_);
        this->t_ = nullptr;
    }
    
    void detach()
    {
        P::thread_detach(this->t_);
        this->t_ = nullptr;
    }
    
    static derived_type ptr_fork(void (*func)(void*), void* ptr)
    {
        // Correct the return type; it's always ignored.
        const auto f = reinterpret_cast<void* (*)(void*)>(func);
        
        return derived_type{ P::thread_create(f, ptr) };
    }
    
private:
    thread_ptr_type t_ = nullptr;
};

} // namespace cmpth

