
#pragma once

#include <mgbase/thread.hpp>
#include <mgbase/atomic.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/assert.hpp>

class bench_master
{
public:
    bench_master()
        : finished_{false}
        , num_threads_{1} { }
    
    virtual ~bench_master() MGBASE_EMPTY_DEFINITION
    
    mgbase::size_t get_num_threads() {
        return num_threads_;
    }
    void set_num_threads(const mgbase::size_t num_threads) {
        MGBASE_ASSERT(!finished_.load());
        num_threads_ = num_threads;
    }
    
    void start()
    {
        finished_.store(false);
        
        ths_ = new mgbase::thread[num_threads_];
        for (mgbase::size_t i = 0; i < num_threads_; ++i)
            ths_[i] = mgbase::thread{ starter{*this, i} };
    }
    
    void finish()
    {
        finished_.store(true, mgbase::memory_order_release);
        
        for (mgbase::size_t i = 0; i < num_threads_; ++i)
            ths_[i].join();
    }
    
protected:
    virtual void thread_main(mgbase::size_t thread_id) = 0;
    
    bool finished() const {
        return finished_.load(mgbase::memory_order_relaxed); // TODO
    }
    
private:
    struct starter {
        bench_master& self;
        mgbase::size_t thread_id;
        
        void operator () () const {
            self.thread_main(thread_id);
        }
    };
    
    mgbase::atomic<bool> finished_;
    mgbase::size_t num_threads_;
    mgbase::scoped_ptr<mgbase::thread []> ths_;
};

