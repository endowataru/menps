
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/type_traits/result_of.hpp>

namespace mgdsm {

template <typename Policy>
class basic_protector_segment_accessor
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::page_id_type           page_id_type;
    typedef typename Policy::block_accessor_type    block_accessor_type;
    
public:
    template <typename Func>
    void do_for_all_blocks_in(const mgbase::uintptr_t idx_in_seg, const mgbase::size_t size_in_bytes, Func func)
    {
        auto& self = this->derived();
        
        const auto first_pg_info = this->get_page_pos(idx_in_seg);
        
        // The last page includes (pos + size - 1).
        const auto last_pg_info = this->get_page_pos(idx_in_seg + size_in_bytes - 1);
        
        // Iterate over [first, last], not [first, last).
        for (page_id_type pg_id = first_pg_info.pg_id; pg_id <= last_pg_info.pg_id; ++pg_id)
        {
            mgbase::uintptr_t from_in_pg = 0;
            if (pg_id == first_pg_info.pg_id)
                from_in_pg = first_pg_info.idx_in_pg;
            
            mgbase::uintptr_t to_in_pg = self.get_page_size();
            if (pg_id == last_pg_info.pg_id)
                to_in_pg = last_pg_info.idx_in_pg + 1; // Be careful
            
            auto pg_ac = self.get_page_accessor(pg_id);
            
            pg_ac.do_for_all_blocks_in(from_in_pg, to_in_pg - from_in_pg, mgbase::forward<Func>(func));
        }
    }
    
    template <typename Func>
    typename mgbase::result_of<Func (block_accessor_type&)>::type
    do_for_block_at(const mgbase::uintptr_t idx_in_seg, Func&& func)
    {
        auto& self = this->derived();
        const auto pg_info = this->get_page_pos(idx_in_seg);
        
        auto pg_ac = self.get_page_accessor(pg_info.pg_id);
        
        return pg_ac.do_for_block_at(pg_info.idx_in_pg, mgbase::forward<Func>(func));
    }
    
private:
    struct page_pos {
        page_id_type        pg_id;
        mgbase::uintptr_t   idx_in_pg;
    };
    
    page_pos get_page_pos(const mgbase::uintptr_t idx_in_seg) const
    {
        auto& self = this->derived();
        const auto pg_size = self.get_page_size();
        
        return {
            static_cast<page_id_type>(idx_in_seg / pg_size)
        ,   idx_in_seg % pg_size
        };
    }
};

} // namespace mgdsm

