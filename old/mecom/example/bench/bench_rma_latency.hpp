
#pragma once

#include "bench_master.hpp"
#include <menps/mecom/rma.hpp>
#include <menps/mecom/collective.hpp>
#include <menps/mecom/structure/alltoall_buffer.hpp>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>

class bench_rma_latency
    : public bench_master
{
    typedef bench_master        base;
    typedef mefdn::uint64_t    value_type;
    typedef mefdn::average_accumulator<mefdn::cpu_clock_t>
        accumulator_type;
    
public:
    bench_rma_latency()
        : msg_size_{1}
        , num_startup_samples_{0} { }
    
    struct thread_info {
        mefdn::size_t      count;
        accumulator_type    overhead;
        accumulator_type    latency;
    };
    
    void collective_setup()
    {
        info_ = mefdn::make_unique<thread_info []>(this->get_num_threads());
        
        lptr_ = mecom::rma::allocate<value_type>(msg_size_);
        buf_.collective_initialize(lptr_);
    }
    void finish()
    {
        base::finish();
        
        mecom::rma::deallocate(lptr_);
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
        info.count = 0;
        
        auto lptr = mecom::rma::allocate<value_type>(msg_size_);
        
        while (!this->finished())
        {
            mefdn::atomic<bool> flag{false};
            
            const auto proc = select_target_proc();
            
            const auto t0 = mefdn::get_cpu_clock();
            
            const auto r = mecom::rma::async_read(
                proc
            ,   buf_.at_process(proc)
            ,   lptr
            ,   msg_size_
            ,   mefdn::make_callback_store_release(&flag, MEFDN_NONTYPE(true))
            );
            
            const auto t1 = mefdn::get_cpu_clock();
            
            if (!r.is_ready()) {
                while (!flag.load(mefdn::memory_order_acquire)) {
                    ult::this_thread::yield();
                }
            }
            
            const auto t2 = mefdn::get_cpu_clock();
            
            if (++info.count > num_startup_samples_)
            {
                info.overhead.add(t1 - t0);
                info.latency.add(t2 - t0);
            }
        }
        
        mecom::rma::deallocate(lptr);
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

