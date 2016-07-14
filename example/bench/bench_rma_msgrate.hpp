
#pragma once

#include "bench_master.hpp"
#include <mgcom/rma.hpp>
#include <mgcom/collective.hpp>
#include <mgcom/structure/alltoall_buffer.hpp>
#include <mgbase/profiling/average_accumulator.hpp>
#include <mgbase/profiling/clock.hpp>
#include <vector>

class bench_rma_msgrate
    : public bench_master
{
    typedef bench_master        base;
    typedef mgbase::uint64_t    value_type;
    typedef mgbase::average_accumulator<mgbase::cpu_clock_t>
        accumulator_type;
    
public:
    bench_rma_msgrate()
        : msg_size_{1}
        , num_startup_samples_{0} { }
    
    struct thread_info {
        mgbase::size_t      count;
        accumulator_type    overhead;
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
        const mgbase::size_t num_local_bufs = 2048;
        
        auto& info = info_[thread_id];
        
        std::vector< mgcom::rma::local_ptr<value_type> > lptrs;
        for (mgbase::size_t i = 0; i < num_local_bufs; ++i)
            lptrs.push_back(mgcom::rma::allocate<value_type>(msg_size_));
        
        mgbase::atomic<mgbase::uint64_t> count{0};
        mgbase::uint64_t posted{0};
        
        while (!this->finished())
        {
            const auto proc = select_target_proc();
            
            while (!this->finished())
            {
                const auto t0 = mgbase::get_cpu_clock();
                
                const bool ret = mgcom::rma::try_read_async(
                    proc
                ,   buf_.at_process(proc)
                ,   lptrs[posted % lptrs.size()]
                ,   msg_size_
                ,   mgbase::make_operation_fetch_add_release(&count, static_cast<mgbase::uint64_t>(1))
                );
                
                const auto t1 = mgbase::get_cpu_clock();
                
                if (ret)
                {
                    if (++posted > num_startup_samples_)
                    {
                        info.overhead.add(t1 - t0);
                    }
                    break;
                }
                else {
                    mgbase::ult::this_thread::yield();
                }
            }
        }
        
        info.count = count.load();
        
        while (count.load() < posted)
            mgbase::ult::this_thread::yield();
        
        for (auto itr = lptrs.begin(); itr != lptrs.end(); ++itr)
            mgcom::rma::deallocate(*itr);
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

