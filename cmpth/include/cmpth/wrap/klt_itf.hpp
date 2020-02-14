
#pragma once

#include <cmpth/wrap/atomic_itf_base.hpp>
#include <cmpth/ult_tag.hpp>
#include <condition_variable>

namespace cmpth {

struct klt_itf
    : atomic_itf_base
{
    using thread = std::thread;
    
    using mutex = std::mutex;
    using condition_variable = std::condition_variable;
    
    using spinlock = std::mutex; // TODO
    
    template <typename Mutex>
    using lock_guard = std::lock_guard<Mutex>;
    template <typename Mutex>
    using unique_lock = std::unique_lock<Mutex>;
    
    template <typename P>
    class thread_specific
    {
        using value_type = typename P::value_type;
        
    public:
        CMPTH_NOINLINE
        value_type* get() const noexcept {
            return this->p_;
        }
        
        CMPTH_NOINLINE
        void set(value_type* const p) const noexcept {
            this->p_ = p;
        }
        
    private:
        static thread_local value_type* p_;
    };
    
    struct this_thread
    {
        static void yield() noexcept {
            std::this_thread::yield();
        }
        
        static thread::id get_id() noexcept {
            return std::this_thread::get_id();
        }
        static pthread_t native_handle() noexcept {
            return pthread_self();
        }
    };
    
    struct initializer {
        initializer() { }
        explicit initializer(int /*argc*/, char** /*argv*/) { }
    };

    using assert_aspect = def_assert_aspect;
    using log_aspect = def_log_aspect;
};

template <typename P>
thread_local typename P::value_type* klt_itf::thread_specific<P>::p_ = nullptr;

template <>
struct get_ult_itf_type<ult_tag_t::KLT>
    : fdn::type_identity<klt_itf> { };

} // namespace cmpth

