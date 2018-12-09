
#pragma once

#include <menps/mefdn/crtp_base.hpp>

namespace menps {
namespace medsm {

template <typename Policy>
class basic_rpc_manager_segment_accessor
    : public mefdn::crtp_base<Policy>
{
    typedef typename Policy::derived_type       derived_type;
    
public:
    mefdn::size_t get_page_size()
    {
        auto& self = this->derived();
        auto& seg = self.get_segment_shard();
        
        return seg.get_page_size();
    }
    
    mefdn::size_t get_num_pages()
    {
        auto& self = this->derived();
        auto& seg = self.get_segment_shard();
        
        return seg.get_num_pages();
    }
    
    mefdn::size_t get_block_size()
    {
        auto& self = this->derived();
        auto& seg = self.get_segment_shard();
        
        return seg.get_block_size();
    }
};

} // namespace medsm
} // namespace menps

