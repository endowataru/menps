
#pragma once

#include "bench_master.hpp"
#include <mgcom/rma.hpp>
#include <mgcom/collective.hpp>
#include <mgcom/structure/alltoall_buffer.hpp>
#include <mgbase/profiling/average_accumulator.hpp>
#include <mgbase/profiling/clock.hpp>

class bench_rma_latency
    : public bench_master
{
    typedef bench_master        base;
    typedef mgbase::uint64_t    value_type;
    typedef mgbase::average_accumulator<mgbase::cpu_clock_t>
        accumulator_type;
    
public:
    bench_rma_latency()
        : msg_size_{1}
        , num_startup_samples_{0} { }
    
    struct thread_info {
        mgbase::size_t      count;
        accumulator_type    overhead;
        accumulator_type    latency;
    };
    
    void collective_setup()
    {
        info_ = new thread_info[this->get_num_threads()];
        
        lptr_ = mgcom::rma::allocate<value_type>(msg_size_);
        buf_.collective_initialize(lptr_);
    }
    void finish()
    {
        base::finish();
        
        mgcom::rma::deallocate(lptr_);
    }
    
    const thread_info& get_thread_info(const mgbase::size_t thread_id) {
        return info_[thread_id];
    }
    
    void set_msg_size(const mgbase::size_t size_in_bytes) {
        msg_size_ = size_in_bytes / sizeof(value_type);
    }
    
    void set_num_startup_samples(const mgbase::size_t num_startup_samples) {
        num_startup_samples_ = num_startup_samples;
    }
    void set_target_proc(const mgcom::process_id_t target_proc) {
        target_proc_ = target_proc;
    }
    
protected:
    virtual void thread_main(const mgbase::size_t thread_id) MGBASE_OVERRIDE
    {
        auto& info = info_[thread_id];
        info.count = 0;
        
        auto lptr = mgcom::rma::allocate<value_type>(msg_size_);
        
        while (!this->finished())
        {
            mgbase::atomic<bool> flag{false};
            
            const auto proc = select_target_proc();
            
            const auto t0 = mgbase::get_cpu_clock();
            
            mgcom::rma::read_async(
                proc
            ,   buf_.at_process(proc)
            ,   lptr
            ,   msg_size_
            ,   mgbase::make_callback_store_release(&flag, MGBASE_NONTYPE(true))
            );
            
            const auto t1 = mgbase::get_cpu_clock();
            
            while (!flag.load(mgbase::memory_order_acquire)) {
                mgbase::ult::this_thread::yield();
            }
            
            const auto t2 = mgbase::get_cpu_clock();
            
            if (++info.count > num_startup_samples_)
            {
                info.overhead.add(t1 - t0);
                info.latency.add(t2 - t0);
            }
        }
        
        mgcom::rma::deallocate(lptr);
    }
    
    virtual mgcom::process_id_t select_target_proc() {
        return target_proc_;
    }
    
private:
    mgcom::rma::local_ptr<value_type>               lptr_;
    mgcom::structure::alltoall_buffer<value_type>   buf_;
    mgbase::scoped_ptr<thread_info []>              info_;
    mgbase::size_t                                  msg_size_;
    mgbase::size_t                                  num_startup_samples_;
    mgcom::process_id_t                             target_proc_;
};

