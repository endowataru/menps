
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meult {

template <typename P>
class uncond_qdlock_thread
{
    using ult_itf_type = typename P::ult_itf_type;
    
    using thread_type = typename ult_itf_type::thread;
    using uncond_variable_type = typename ult_itf_type::uncond_variable;
    
    using atomic_bool_type = typename P::atomic_bool_type;

public:
    uncond_qdlock_thread()
        : finished_(false)
    { }
    
    ~uncond_qdlock_thread()
    {
        this->stop();
    }
    
    template <typename Func>
    void start(Func&& func)
    {
        th_ = thread_type(
            thread_main<mefdn::decay_t<Func>>{
                *this
            ,   mefdn::forward<Func>(func)
            }
        );
    }
    
    void set_finished()
    {
        this->finished_.store(true);
    }
    
    void stop()
    {
        if (th_.joinable())
        {
            MEFDN_ASSERT(this->finished_.load());
            
            this->th_.join();
            
            this->finished_.store(false);
        }
    }
    
    uncond_variable_type& get_uv() noexcept {
        return this->uv_;
    }
    
private:
    template <typename Func>
    struct thread_main
    {
        uncond_qdlock_thread&   self;
        Func                    func;
        
        void operator() ()
        {
            self.uv_.wait();
            
            while (!self.finished_.load(mefdn::memory_order_relaxed))
            {
                func();
            }
        }
    };
    
    atomic_bool_type finished_;
    uncond_variable_type uv_;
    thread_type th_;
};

} // namespace meult
} // namespace menps

