
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/atomic.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class rel_thread
{
    using ult_itf_type = typename P::ult_itf_type;
    using thread_type = typename ult_itf_type::thread;
    
    using usec_type = useconds_t; // TODO
    
public:
    template <typename Func>
    void start(Func func, const usec_type usec) {
        this->finished_.store(false);
        this->sleep_usec_ = usec;
        this->th_ = thread_type(worker_main<Func>{ *this, func });
    }
    
    void stop() {
        this->finished_.store(true);
        this->th_.join();
    }
    
private:
    template <typename Func>
    struct worker_main
    {
        rel_thread& self;
        Func        func;
        
        void operator() ()
        {
            while (!self.finished_.load(mefdn::memory_order_relaxed))
            {
                #if 1
                myth_usleep(self.sleep_usec_);
                #else
                const auto from = mefdn::get_cpu_clock();
                while (mefdn::get_cpu_clock() < from + self.sleep_usec_) {
                    myth_yield();
                }
                #endif
                
                func();
            }
        }
    };
    
    mefdn::atomic<bool> finished_;
    thread_type         th_;
    usec_type           sleep_usec_;
};

} // namespace medsm2
} // namespace menps

