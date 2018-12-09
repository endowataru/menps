
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
        info_ = new thread_info[this->get_num_threads()];
        
        lptr_ = mecom::rma::allocate<value_type>(msg_size_);
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
        // TODO
        const mefdn::size_t num_local_bufs = mefdn::min(
            (1ull << 30) / this->get_num_threads() / msg_size_ / sizeof(value_type)
        ,   16ull << 10
        );
        MEFDN_LOG_DEBUG(
            "msg:Allocating buffers.\t"
            "num_local_bufs:{}\tnum_threads:{}\tmsg_size:{}"
        ,   num_local_bufs
        ,   this->get_num_threads()
        ,   msg_size_
        );
        
        auto& info = info_[thread_id];
        
        const auto large_buf
            = mecom::rma::allocate<value_type>(msg_size_ * num_local_bufs);
        
        std::vector< mecom::rma::local_ptr<value_type> > lptrs;
        for (mefdn::size_t i = 0; i < num_local_bufs; ++i) {
            lptrs.push_back(large_buf + i * msg_size_);
            //lptrs.push_back(mecom::rma::allocate<value_type>(msg_size_));
        }
        
        mefdn::unique_ptr<mefdn::atomic<bool> []> flags{new mefdn::atomic<bool>[num_local_bufs]};
        for (mefdn::size_t i = 0; i < num_local_bufs; ++i)
            flags[i].store(true);
        
        /*mefdn::atomic<mefdn::uint64_t> count{0};
        mefdn::uint64_t posted{0};*/
        
        mefdn::uint64_t pos = 0;
        mefdn::int64_t count = -num_local_bufs;
        
        while (!this->finished())
        {
            const auto proc = select_target_proc();
            
            while (!flags[pos].load(mefdn::memory_order_acquire)) {
                ult::this_thread::yield();
                /*busy loop*/
            }
            
            flags[pos].store(false);
            ++count;
            
            const auto t0 = mefdn::get_cpu_clock();
            
            const auto r = mecom::rma::async_read(
                proc
            ,   buf_.at_process(proc)
            ,   lptrs[pos]
            ,   msg_size_
            ,   mefdn::make_callback_store_release(&flags[pos], MEFDN_NONTYPE(true))
            );
            /*const bool ret = mecom::rma::try_read_async(
                proc
            ,   buf_.at_process(proc)
            ,   lptrs[posted % lptrs.size()]
            ,   msg_size_
            ,   mefdn::make_callback_fetch_add_release(&count, MEFDN_NONTYPE(1))
            );*/
            
            const auto t1 = mefdn::get_cpu_clock();
            
            if (count > num_startup_samples_)
            {
                info.overhead.add(t1 - t0);
            }
            
            pos = (pos + 1) % num_local_bufs;
            
            if (r.is_ready()) {
                flags[pos].store(true, mefdn::memory_order_relaxed);
            }
            
            //while (posted - count.load() >= lptrs.size()) {
                // busy loop
                //mefdn::ult::this_thread::yield();
            //}
                
            #if 0
            if (posted % lptrs.size() == 0)
            {
                while (count.load() < posted) { /*busy loop*/ }
            }
            #endif
        }
        
        for (mefdn::size_t i = 0; i < num_local_bufs; ++i) {
            if (flags[i].load(mefdn::memory_order_relaxed))
                ++count;
        }
        
        info.count = count;
        
        #if 0 // TODO : disable waiting for all the completions (to get the results fast)
        // Wait for all established requests.
        for (mefdn::size_t i = 0; i < num_local_bufs; ++i) {
            while (!flags[i].load(mefdn::memory_order_relaxed))
                mecom::ult::this_thread::yield();
        }
        #endif
        
        // TODO : memory leak
        flags.release();
        
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
    mefdn::scoped_ptr<thread_info []>              info_;
    mefdn::size_t                                  msg_size_;
    mefdn::size_t                                  num_startup_samples_;
    mecom::process_id_t                             target_proc_;
};

