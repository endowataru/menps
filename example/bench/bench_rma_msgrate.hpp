
#pragma once

#include "bench_master.hpp"
#include <mgcom/rma.hpp>
#include <mgcom/collective.hpp>
#include <mgcom/structure/alltoall_buffer.hpp>
#include <mgbase/profiling/average_accumulator.hpp>
#include <mgbase/profiling/clock.hpp>
#include <mgbase/algorithm.hpp>
#include <mgbase/logger.hpp>
#include <mgbase/unique_ptr.hpp>
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
        
        #if 0 // TODO : disable waiting for all the completions (to get the results fast)
        mgcom::rma::deallocate(lptr_);
        #endif
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
        // TODO
        const mgbase::size_t num_local_bufs = mgbase::min(
            (1ull << 30) / this->get_num_threads() / msg_size_ / sizeof(value_type)
        ,   16ull << 10
        );
        MGBASE_LOG_DEBUG(
            "msg:Allocating buffers.\t"
            "num_local_bufs:{}\tnum_threads:{}\tmsg_size:{}"
        ,   num_local_bufs
        ,   this->get_num_threads()
        ,   msg_size_
        );
        
        auto& info = info_[thread_id];
        
        const auto large_buf
            = mgcom::rma::allocate<value_type>(msg_size_ * num_local_bufs);
        
        std::vector< mgcom::rma::local_ptr<value_type> > lptrs;
        for (mgbase::size_t i = 0; i < num_local_bufs; ++i) {
            lptrs.push_back(large_buf + i * msg_size_);
            //lptrs.push_back(mgcom::rma::allocate<value_type>(msg_size_));
        }
        
        mgbase::unique_ptr<mgbase::atomic<bool> []> flags{new mgbase::atomic<bool>[num_local_bufs]};
        for (mgbase::size_t i = 0; i < num_local_bufs; ++i)
            flags[i].store(true);
        
        /*mgbase::atomic<mgbase::uint64_t> count{0};
        mgbase::uint64_t posted{0};*/
        
        mgbase::uint64_t pos = 0;
        mgbase::int64_t count = -num_local_bufs;
        
        while (!this->finished())
        {
            const auto proc = select_target_proc();
            
            while (!flags[pos].load(mgbase::memory_order_acquire)) { /*busy loop*/ }
            
            flags[pos].store(false);
            ++count;
            
            const auto t0 = mgbase::get_cpu_clock();
            
            const auto r = mgcom::rma::async_read(
                proc
            ,   buf_.at_process(proc)
            ,   lptrs[pos]
            ,   msg_size_
            ,   mgbase::make_callback_store_release(&flags[pos], MGBASE_NONTYPE(true))
            );
            /*const bool ret = mgcom::rma::try_read_async(
                proc
            ,   buf_.at_process(proc)
            ,   lptrs[posted % lptrs.size()]
            ,   msg_size_
            ,   mgbase::make_callback_fetch_add_release(&count, MGBASE_NONTYPE(1))
            );*/
            
            const auto t1 = mgbase::get_cpu_clock();
            
            if (count > num_startup_samples_)
            {
                info.overhead.add(t1 - t0);
            }
            
            pos = (pos + 1) % num_local_bufs;
            
            if (r.is_ready()) {
                flags[pos].store(true, mgbase::memory_order_relaxed);
            }
            
            //while (posted - count.load() >= lptrs.size()) {
                // busy loop
                //mgbase::ult::this_thread::yield();
            //}
                
            #if 0
            if (posted % lptrs.size() == 0)
            {
                while (count.load() < posted) { /*busy loop*/ }
            }
            #endif
        }
        
        for (mgbase::size_t i = 0; i < num_local_bufs; ++i) {
            if (flags[i].load(mgbase::memory_order_relaxed))
                ++count;
        }
        
        info.count = count;
        
        #if 0 // TODO : disable waiting for all the completions (to get the results fast)
        // Wait for all established requests.
        for (mgbase::size_t i = 0; i < num_local_bufs; ++i) {
            while (!flags[i].load(mgbase::memory_order_relaxed))
                mgcom::ult::this_thread::yield();
        }
        #endif
        
        // TODO : memory leak
        flags.release();
        
        /*info.count = count.load();
        
        */
        
        // TODO: currently deallocation is disabled (= memory leak)
        /*for (auto itr = lptrs.begin(); itr != lptrs.end(); ++itr)
            mgcom::rma::deallocate(*itr);*/
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

