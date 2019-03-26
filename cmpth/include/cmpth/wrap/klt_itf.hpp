
#pragma once

#include <cmpth/common.hpp>

namespace cmpth {

struct klt_itf
{
    using thread = std::thread;
    
    using mutex = std::mutex;
    
    using spinlock = std::mutex; // TODO
    
    template <typename Mutex>
    using unique_lock = std::unique_lock<Mutex>;
    
    template <typename T>
    using atomic = std::atomic<T>;
    
    template <typename P>
    class thread_specific
    {
        using value_type = typename P::value_type;
        
    public:
        value_type* get() const noexcept {
            return p_;
        }
        
        void set(value_type* const p) const noexcept {
            this->p_ = p;
        }
        
    private:
        static thread_local value_type* p_;
    };
};

template <typename P>
thread_local typename P::value_type* klt_itf::thread_specific<P>::p_ = nullptr;

} // namespace cmpth

