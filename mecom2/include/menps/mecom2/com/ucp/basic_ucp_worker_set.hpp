
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/medev2/ucx/ucp/ucp.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_ucp_worker_set
{
    using ucp_itf_type = typename P::ucp_itf_type;
    using worker_type = typename ucp_itf_type::worker_type;
    using worker_address_type = typename ucp_itf_type::worker_address_type;
    using endpoint_type = typename ucp_itf_type::endpoint_type;
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    using worker_num_type = typename P::worker_num_type;
    
public:
    template <typename Conf>
    explicit basic_ucp_worker_set(const Conf& conf)
    {
        auto& uf = conf.uf;
        auto& ctx = conf.ctx;
        auto& coll = conf.coll;
        const auto num_procs = coll.get_num_procs();
        
        const auto num_wks = conf.num_wks;
        this->num_wks_ = num_wks;
        
        wis_ = mefdn::make_unique<worker_info []>(num_wks);
        
        size_type cur_addr_size = 0;
        
        for (worker_num_type wk_num = 0; wk_num < num_wks; ++wk_num)
        {
            auto& wi = wis_[wk_num];
            
            // Create a worker.
            wi.wk = worker_type::create(uf, ctx.get(), &conf.wk_params);
            wi.wk_addr = worker_address_type::get_address(uf, wi.wk.get());
            
            const auto sz = wi.wk_addr.size_in_bytes();
            cur_addr_size = std::max(cur_addr_size, sz);
        }
        
        size_type max_addr_size = 0;
        coll.allreduce_max(&cur_addr_size, &max_addr_size, 1);
        
        const auto cur_addr_buf =
            mefdn::make_unique<mefdn::byte []>(num_wks * max_addr_size);
        
        for (worker_num_type wk_num = 0; wk_num < num_wks; ++wk_num)
        {
            auto& wi = wis_[wk_num];
            const auto sz = wi.wk_addr.size_in_bytes();
            
            std::memcpy(&cur_addr_buf[wk_num * max_addr_size], wi.wk_addr.get(), sz);
        }
        
        const auto all_addr_buf =
            mefdn::make_unique<mefdn::byte []>(num_procs * num_wks * max_addr_size);
        
        coll.allgather(cur_addr_buf.get(), all_addr_buf.get(), num_wks * max_addr_size);
        
        for (worker_num_type wk_num = 0; wk_num < num_wks; ++wk_num)
        {
            auto& wi = wis_[wk_num];
            wi.eps = mefdn::make_unique<endpoint_type []>(num_procs);
            
            for (proc_id_type proc = 0; proc < num_procs; ++proc)
            {
                ucp_ep_params_t ep_params = ucp_ep_params_t();
                ep_params.field_mask = UCP_EP_PARAM_FIELD_REMOTE_ADDRESS;
                ep_params.address = reinterpret_cast<const ucp_address_t*>(
                    &all_addr_buf[(proc * num_wks + wk_num) * max_addr_size]
                );
                
                // [p=0,w=0], [p=0,w=1], ..., [p=0,w=num_wks-1], [p=1,w=0], ...
                
                wi.eps[proc] = endpoint_type::create(uf, wi.wk.get(), &ep_params);
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
    
    endpoint_type& get_ep(
        const worker_num_type   wk_num
    ,   const proc_id_type      proc
    ) {
        return this->wis_[wk_num].eps[proc];
    }
    
private:
    struct worker_info {
        worker_type                         wk;
        worker_address_type                 wk_addr;
        mefdn::unique_ptr<endpoint_type []> eps;
            // indexed by process ID
    };
    
    mefdn::unique_ptr<worker_info []> wis_;
        // indexed by worker index
    
    size_type num_wks_ = 0;
};

} // namespace mecom2
} // namespace menps

