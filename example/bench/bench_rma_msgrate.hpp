
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

template <bool Write>
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
        
        lptr_ = mgcom::rma::allocate<value_type>(msg_size_ * this->get_num_threads() * 8);
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
        auto& info = info_[thread_id];
        
        const auto lbuf = mgcom::rma::allocate<value_type>(msg_size_);
        
        const mgbase::size_t num_batch = 4096; // TODO
        #if 0
        
        const auto flags =
            mgbase::make_unique<mgbase::atomic<bool> []>(num_batch);
        
        for (mgbase::size_t i = 0; i < num_batch; ++i)
            flags[i].store(true);
        
        mgbase::uint64_t pos = 0;
        mgbase::int64_t count = -num_local_bufs;
        #endif
        // num_finished{0};
        auto num_finished = mgbase::make_unique<mgbase::atomic<mgbase::uint64_t>>();
        num_finished->store(0);
        mgbase::uint64_t num_established = 0;
        
        while (!this->finished())
        {
            /*while (num_established - num_finished.load(mgbase::memory_order_relaxed) > num_batch) {
                ult::this_thread::yield();
            }*/
            
            const auto proc = select_target_proc();
            
            
            #if 0
            while (!flags[pos].load(mgbase::memory_order_acquire)) {
                ult::this_thread::yield();
                /*busy loop*/
            }
            
            flags[pos].store(false);
            ++count;
            #endif
            
            const auto t0 = mgbase::get_cpu_clock();
            
            bool is_ready;
            
            if (Write) {
                const auto r = mgcom::rma::async_write(
                    proc
                ,   buf_.at_process(proc) + this->get_num_threads() * 8
                ,   lbuf
                ,   msg_size_
                ,   mgbase::make_callback_fetch_add_release(num_finished.get(), MGBASE_NONTYPE(1))
                );
                
                is_ready = r.is_ready();
            } else {
                const auto r = mgcom::rma::async_read(
                    proc
                ,   buf_.at_process(proc) + this->get_num_threads() * 8
                ,   lbuf
                ,   msg_size_
                ,   mgbase::make_callback_fetch_add_release(num_finished.get(), MGBASE_NONTYPE(1))
                );
                
                is_ready = r.is_ready();
            }
            
            const auto t1 = mgbase::get_cpu_clock();
            
            if (num_established > num_startup_samples_)
            {
                info.overhead.add(t1 - t0);
            }
            
            #if 0
            pos = (pos + 1) % num_local_bufs;
            #endif
            
            ++num_established;
            
            if (is_ready) {
                num_finished->fetch_add(1, mgbase::memory_order_relaxed);
                //flags[pos].store(true, mgbase::memory_order_relaxed);
            }
        }
        
        #if 0
        for (mgbase::size_t i = 0; i < num_local_bufs; ++i) {
            if (flags[i].load(mgbase::memory_order_relaxed))
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
        for (mgbase::size_t i = 0; i < num_local_bufs; ++i) {
            while (!flags[i].load(mgbase::memory_order_relaxed))
                mgcom::ult::this_thread::yield();
        }
        #endif
        
        
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

