
#pragma once

#include <mgbase/crtp_base.hpp>

namespace mgdsm {

template <typename Policy>
class basic_rpc_manager_segment_accessor
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::derived_type       derived_type;
    
public:
    mgbase::size_t get_page_size()
    {
        auto& self = this->derived();
        auto& seg = self.get_segment_shard();
        
        return seg.get_page_size();
    }
    
    mgbase::size_t get_num_pages()
    {
        auto& self = this->derived();
        auto& seg = self.get_segment_shard();
        
        return seg.get_num_pages();
    }
    
    mgbase::size_t get_block_size()
    {
        auto& self = this->derived();
        auto& seg = self.get_segment_shard();
        
        return seg.get_block_size();
    }
};

} // namespace mgdsm

