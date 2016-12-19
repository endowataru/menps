
#pragma once

#include <mgbase/crtp_base.hpp>

namespace mgdsm {

template <typename Policy>
class basic_rpc_manager_segment_accessor
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::derived_type       derived_type;
    typedef typename Policy::process_id_type    process_id_type;
    typedef typename Policy::page_id_type       page_id_type;
    
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
    
    void enable_flush(const process_id_type proc, const page_id_type pg_id)
    {
        auto& self = this->derived();
        
        const auto seg_id = self.get_segment_id();
        auto& sp = self.get_space();
        
        auto& actv = sp.get_activater();
        
        actv.enable_flush(proc, seg_id, pg_id);
    }
    
    void enable_diff(const process_id_type proc, const page_id_type pg_id)
    {
        auto& self = this->derived();
        
        const auto seg_id = self.get_segment_id();
        auto& sp = self.get_space();
        
        auto& actv = sp.get_activater();
        
        actv.enable_diff(proc, seg_id, pg_id);
    }
};

} // namespace mgdsm

