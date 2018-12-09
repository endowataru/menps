
#pragma once

#include <menps/mefdn/thread.hpp>
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/assert.hpp>

#include <menps/mecom/ult.hpp>

namespace mefdn = menps::mefdn;
namespace mecom = menps::mecom;

namespace ult = mecom::ult;

class bench_master
{
public:
    bench_master()
        : finished_{false}
        , num_threads_{1} { }
    
    virtual ~bench_master() /*noexcept*/ = default;
    
    mefdn::size_t get_num_threads() {
        return num_threads_;
    }
    void set_num_threads(const mefdn::size_t num_threads) {
        MEFDN_ASSERT(!finished_.load());
        num_threads_ = num_threads;
    }
    
    void start()
    {
        finished_.store(false);
        
        ths_ = mefdn::make_unique<ult::thread []>(num_threads_);
        for (mefdn::size_t i = 0; i < num_threads_; ++i)
            ths_[i] = ult::thread{ starter{*this, i} };
    }
    
    void finish()
    {
        finished_.store(true, mefdn::memory_order_release);
        
        for (mefdn::size_t i = 0; i < num_threads_; ++i)
            ths_[i].join();
    }
    
protected:
    virtual void thread_main(mefdn::size_t thread_id) = 0;
    
    bool finished() const {
        return finished_.load(mefdn::memory_order_relaxed); // TODO
    }
    
private:
    struct starter {
        bench_master& self;
        mefdn::size_t thread_id;
        
        void operator () () const {
            self.thread_main(thread_id);
        }
    };
    
    mefdn::atomic<bool> finished_;
    mefdn::size_t num_threads_;
    mefdn::unique_ptr<ult::thread []> ths_;
};

