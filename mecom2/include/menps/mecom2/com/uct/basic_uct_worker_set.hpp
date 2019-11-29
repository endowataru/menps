
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/medev2/ucx/uct/uct.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_uct_worker_set
{
    using uct_itf_type = typename P::uct_itf_type;
    using async_context_type = typename uct_itf_type::async_context_type;
    using worker_type = typename uct_itf_type::worker_type;
    using interface_type = typename uct_itf_type::interface_type;
    using device_address_type = typename uct_itf_type::device_address_type;
    using endpoint_type = typename uct_itf_type::endpoint_type;
    using ep_address_type = typename uct_itf_type::ep_address_type;
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    using worker_num_type = typename P::worker_num_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using spinlock_type = typename ult_itf_type::spinlock;
    
public:
    template <typename Conf>
    explicit basic_uct_worker_set(const Conf& conf)
    {
        auto& uf = conf.uf;
        auto& md = conf.md;
        const auto* const iface_conf = conf.iface_conf;
        const auto* const iface_params = conf.iface_params;
        
        auto& coll = conf.coll;
        const auto this_proc = coll.this_proc_id();
        const auto num_procs = coll.get_num_procs();
        
        const auto num_wks = conf.num_wks;
        this->num_wks_ = num_wks;
        
        this->wis_ = mefdn::make_unique<worker_info []>(num_wks);
        
        size_type cur_dev_addr_size = 0;
        size_type cur_ep_addr_size = 0;
        
        #ifndef MECOM2_UCT_ENABLE_PARALLEL_INIT_WORKER_SET
        for (proc_id_type init_proc = 0; init_proc < num_procs; ++init_proc)
        {
            // TODO: Initializing the workers inside the same compute node may cause
            //       deadlocks when calling uct_md_open() between the processes.
            //       This code allows only one process to do the initialization at the same time.
            
            coll.barrier();
            
            if (init_proc == this_proc)
            {
        #endif
                for (worker_num_type wk_num = 0; wk_num < num_wks; ++wk_num)
                {
                    auto& wi = this->wis_[wk_num];
                    
                    // Create a UCT async context.
                    wi.async_ctx = async_context_type::create(UCS_ASYNC_MODE_THREAD);
                    // Create a UCT worker.
                    wi.wk = worker_type::create(uf, wi.async_ctx.get(), UCS_THREAD_MODE_SERIALIZED);
                    // Open a UCT interface.
                    wi.iface = interface_type::open(uf, md.get(), wi.wk.get(),
                        iface_params, iface_conf);
                    
                    // Get some information about the UCT interface.
                    wi.iface_attr = wi.iface.query();
                    wi.dev_addr = uct_itf_type::get_device_address(wi.iface, wi.iface_attr);
                    
                    cur_dev_addr_size = std::max(cur_dev_addr_size, wi.dev_addr.size_in_bytes());
                    
                    wi.eis = mefdn::make_unique<endpoint_info []>(num_procs);
                    
                    for (proc_id_type proc = 0; proc < num_procs; ++proc)
                    {
                        auto& ei = wi.eis[proc];
                        
                        ei.ep = endpoint_type::create(uf, wi.iface.get());
                        ei.ep_addr = uct_itf_type::get_ep_address(ei.ep, wi.iface_attr);
                        
                        ei.num_ongoing = 0;
                        ei.max_num_ongoing = MECOM2_UCT_RMA_CONCURRENT_LIMIT_COUNT; // TODO;
                        
                        cur_ep_addr_size = std::max(cur_ep_addr_size, ei.ep_addr.size_in_bytes());
                    }
                }
        #ifndef MECOM2_UCT_ENABLE_PARALLEL_INIT_WORKER_SET
            }
        }
        #endif
        
        size_type max_dev_addr_size = 0;
        coll.allreduce_max(&cur_dev_addr_size, &max_dev_addr_size, 1);
        size_type max_ep_addr_size = 0;
        coll.allreduce_max(&cur_ep_addr_size, &max_ep_addr_size, 1);
        
        const auto cur_dev_addr_buf =
            mefdn::make_unique<mefdn::byte []>(num_wks * max_dev_addr_size);
        const auto cur_ep_addr_buf =
            mefdn::make_unique<mefdn::byte []>(num_procs * num_wks * max_ep_addr_size);
        
        for (worker_num_type wk_num = 0; wk_num < num_wks; ++wk_num)
        {
            auto& wi = this->wis_[wk_num];
            
            std::memcpy(
                &cur_dev_addr_buf[wk_num * max_dev_addr_size]
            ,   wi.dev_addr.get()
            ,   wi.dev_addr.size_in_bytes()
            );
            
            for (proc_id_type proc = 0; proc < num_procs; ++proc)
            {
                auto& ei = wi.eis[proc];
                
                std::memcpy(
                    &cur_ep_addr_buf[(proc * num_wks + wk_num) * max_ep_addr_size]
                ,   ei.ep_addr.get()
                ,   ei.ep_addr.size_in_bytes()
                );
            }
        }
        
        const auto all_dev_addr_buf =
            mefdn::make_unique<mefdn::byte []>(
                num_procs * num_wks * max_dev_addr_size);
        const auto all_ep_addr_buf =
            mefdn::make_unique<mefdn::byte []>(
                num_procs * num_wks * max_ep_addr_size);
        
        // One worker has only one device address.
        coll.allgather(cur_dev_addr_buf.get(), all_dev_addr_buf.get(),
            num_wks * max_dev_addr_size);
        // One worker has N endpoints (and addresses) (N = # of processes).
        coll.alltoall(cur_ep_addr_buf.get(), all_ep_addr_buf.get(),
            num_wks * max_ep_addr_size);
        
        for (worker_num_type wk_num = 0; wk_num < num_wks; ++wk_num)
        {
            auto& wi = this->wis_[wk_num];
            
            for (proc_id_type proc = 0; proc < num_procs; ++proc)
            {
                auto& ei = wi.eis[proc];
            
                auto* remote_dev_addr =
                    reinterpret_cast<const uct_device_addr_t*>(
                        &all_dev_addr_buf[
                            (proc * num_wks + wk_num) * max_dev_addr_size
                        ]
                    );
                
                auto* remote_ep_addr =
                    reinterpret_cast<const uct_ep_addr_t*>(
                        &all_ep_addr_buf[
                            (proc * num_wks + wk_num) * max_ep_addr_size
                        ]
                    );
                
                ei.ep.connect_to_ep(remote_dev_addr, remote_ep_addr);
            }
        }
        
        coll.barrier();
    }
    
    size_type get_num_workers() {
        return this->num_wks_;
    }
    
    worker_type& get_worker(const worker_num_type wk_num)
    {
        return this->wis_[wk_num].wk;
    }
    interface_type& get_iface(const worker_num_type wk_num)
    {
        return this->wis_[wk_num].iface;
    }
    
    endpoint_type& get_ep(
        const worker_num_type   wk_num
    ,   const proc_id_type      proc
    ) {
        return this->wis_[wk_num].eis[proc].ep;
    }
    
    spinlock_type& get_worker_lock(
        const worker_num_type   wk_num
    ) {
        return this->wis_[wk_num].lock;
    }
    
    MEFDN_NODISCARD
    bool try_start_rma(
        const worker_num_type   wk_num MEFDN_MAYBE_UNUSED
    ,   const proc_id_type      proc MEFDN_MAYBE_UNUSED
    ) {
        #ifdef MECOM2_UCT_RMA_ENABLE_CONCURRENT_LIMIT
        auto& ei = this->wis_[wk_num].eis[proc];
        
        mefdn::lock_guard<spinlock_type> lk(ei.lock);
        if (ei.num_ongoing < ei.max_num_ongoing) {
            ++ei.num_ongoing;
            MEFDN_LOG_VERBOSE(
                "msg:Start RMA.\t"
                "num_ongoing:{}\t"
                "max_num_ongoing:{}"
            ,   ei.num_ongoing
            ,   ei.max_num_ongoing
            );
            return true;
        }
        else
            return false;
        #else
        return true;
        #endif
    }
    void finish_rma(
        const worker_num_type   wk_num MEFDN_MAYBE_UNUSED
    ,   const proc_id_type      proc MEFDN_MAYBE_UNUSED
    ) {
        #ifdef MECOM2_UCT_RMA_ENABLE_CONCURRENT_LIMIT
        auto& ei = this->wis_[wk_num].eis[proc];
        
        mefdn::lock_guard<spinlock_type> lk(ei.lock);
        MEFDN_ASSERT(ei.num_ongoing > 0);
        --ei.num_ongoing;
        MEFDN_LOG_VERBOSE(
            "msg:Finish RMA.\t"
            "num_ongoing:{}\t"
            "max_num_ongoing:{}"
        ,   ei.num_ongoing
        ,   ei.max_num_ongoing
        );
        #endif
    }
    
private:
    struct endpoint_info {
        endpoint_type   ep;
        ep_address_type ep_addr;
        
        spinlock_type   lock;
        size_type       num_ongoing;
        size_type       max_num_ongoing;
    };
    
    struct worker_info {
        async_context_type                  async_ctx;
        worker_type                         wk;
        interface_type                      iface;
        uct_iface_attr_t                    iface_attr;
        device_address_type                 dev_addr;
        mefdn::unique_ptr<endpoint_info []> eis;
            // indexed by process ID
        spinlock_type                       lock;
    };
    
    mefdn::unique_ptr<worker_info []> wis_;
        // indexed by worker index
    
    size_type num_wks_ = 0;
};

} // namespace mecom2
} // namespace menps

