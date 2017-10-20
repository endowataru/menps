
#pragma once

#include "bench_master.hpp"
#include <menps/mecom/rma.hpp>
#include <menps/mecom/collective.hpp>
#include <menps/mecom/structure/alltoall_buffer.hpp>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/algorithm.hpp>
#include <menps/mefdn/logger.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <vector>

template <bool Write>
class bench_rma_msgrate
    : public bench_master
{
    typedef bench_master        base;
    typedef mefdn::uint64_t    value_type;
    typedef mefdn::average_accumulator<mefdn::cpu_clock_t>
        accumulator_type;
    
public:
    bench_rma_msgrate()
        : msg_size_{1}
        , num_startup_samples_{0} { }
    
    struct thread_info {
        mefdn::size_t      count;
        accumulator_type    overhead;
    };
    
    void collective_setup()
    {
        info_ = mefdn::make_unique<thread_info []>(this->get_num_threads());
        
        lptr_ = mecom::rma::allocate<value_type>(msg_size_ /** this->get_num_threads()*/);
        buf_.collective_initialize(lptr_);
    }
    void finish()
    {
        base::finish();
        
        #if 0 // TODO : disable waiting for all the completions (to get the results fast)
        mecom::rma::deallocate(lptr_);
        #endif
    }
    
    const thread_info& get_thread_info(const mefdn::size_t thread_id) {
        return info_[thread_id];
    }
    
    void set_msg_size(const mefdn::size_t size_in_bytes) {
        msg_size_ = size_in_bytes / sizeof(value_type);
    }
    void set_num_startup_samples(const mefdn::size_t num_startup_samples) {
        num_startup_samples_ = num_startup_samples;
    }
    void set_target_proc(const mecom::process_id_t target_proc) {
        target_proc_ = target_proc;
    }
    
protected:
    virtual void thread_main(const mefdn::size_t thread_id) MEFDN_OVERRIDE
    {
        auto& info = info_[thread_id];
        
        const auto lbuf = mecom::rma::allocate<value_type>(msg_size_);
        
        const mefdn::size_t num_batch = 256; // TODO
        #if 0
        
        const auto flags =
            mefdn::make_unique<mefdn::atomic<bool> []>(num_batch);
        
        for (mefdn::size_t i = 0; i < num_batch; ++i)
            flags[i].store(true);
        
        mefdn::uint64_t pos = 0;
        mefdn::int64_t count = -num_local_bufs;
        #endif
        // num_finished{0};
        auto num_finished = mefdn::make_unique<mefdn::atomic<mefdn::uint64_t>>();
        num_finished->store(0);
        mefdn::uint64_t num_established = 0;
        
        while (!this->finished())
        {
            while (num_established - num_finished->load(mefdn::memory_order_relaxed) > num_batch) {
                ult::this_thread::yield();
            }
            
            const auto proc = select_target_proc();
            
            #if 0
            while (!flags[pos].load(mefdn::memory_order_acquire)) {
                ult::this_thread::yield();
                /*busy loop*/
            }
            
            flags[pos].store(false);
            ++count;
            #endif
            
            const auto t0 = mefdn::get_cpu_clock();
            
            bool is_ready;
            
            if (Write) {
                const auto r = mecom::rma::async_write(
                    proc
                ,   buf_.at_process(proc)// + thread_id*msg_size_
                ,   lbuf
                ,   msg_size_
                ,   mefdn::make_callback_fetch_add_release(num_finished.get(), MEFDN_NONTYPE(1))
                );
                
                is_ready = r.is_ready();
            } else {
                const auto r = mecom::rma::async_read(
                    proc
                ,   buf_.at_process(proc)// + thread_id*msg_size_
                ,   lbuf
                ,   msg_size_
                ,   mefdn::make_callback_fetch_add_release(num_finished.get(), MEFDN_NONTYPE(1))
                );
                
                is_ready = r.is_ready();
            }
            
            const auto t1 = mefdn::get_cpu_clock();
            
            if (num_established > num_startup_samples_)
            {
                info.overhead.add(t1 - t0);
            }
            
            #if 0
            pos = (pos + 1) % num_local_bufs;
            #endif
            
            ++num_established;
            
            if (is_ready) {
                num_finished->fetch_add(1, mefdn::memory_order_relaxed);
                //flags[pos].store(true, mefdn::memory_order_relaxed);
            }
        }
        
        #if 0
        for (mefdn::size_t i = 0; i < num_local_bufs; ++i) {
            if (flags[i].load(mefdn::memory_order_relaxed))
                ++count;
        }
        #endif
        
        info.count = num_finished->load();
        
        #if 0
        num_finished.release();
        #else
        while (num_finished->load() < num_established)
        {
            ult::this_thread::yield();
        }
        #endif
        
        
        #if 0 // TODO : disable waiting for all the completions (to get the results fast)
        // Wait for all established requests.
        for (mefdn::size_t i = 0; i < num_local_bufs; ++i) {
            while (!flags[i].load(mefdn::memory_order_relaxed))
                mecom::ult::this_thread::yield();
        }
        #endif
        
        
        /*info.count = count.load();
        
        */
        
        // TODO: currently deallocation is disabled (= memory leak)
        /*for (auto itr = lptrs.begin(); itr != lptrs.end(); ++itr)
            mecom::rma::deallocate(*itr);*/
    }
    
    virtual mecom::process_id_t select_target_proc() {
        return target_proc_;
    }
    
private:
    mecom::rma::local_ptr<value_type>               lptr_;
    mecom::structure::alltoall_buffer<value_type>   buf_;
    mefdn::unique_ptr<thread_info []>              info_;
    mefdn::size_t                                  msg_size_;
    mefdn::size_t                                  num_startup_samples_;
    mecom::process_id_t                             target_proc_;
};

