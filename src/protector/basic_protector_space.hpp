
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/unique_ptr.hpp>
#include <mgbase/utility/move.hpp>
#include <mgbase/utility/forward.hpp>
#include <mgbase/type_traits/result_of.hpp>
#include <mgbase/assert.hpp>
#include <vector>

namespace mgdsm {

template <typename Policy>
class basic_protector_space
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::block_accessor_type    block_accessor_type;
    
    typedef typename Policy::segment_type           segment_type;
    typedef typename Policy::segment_id_type        segment_id_type;
    typedef typename Policy::abs_block_id_type      abs_block_id_type;
    
    typedef mgbase::unique_ptr<segment_type>        segment_ptr_type;
    
public:
    template <typename Conf>
    explicit basic_protector_space(const Conf& conf)
        : segs_(mgbase::make_unique<segment_ptr_type []>(conf.num_segments))
    { }
    
    template <typename Func>
    void do_for_all_blocks_in(void* const ptr, const mgbase::size_t size_in_bytes, Func&& func)
    {
        auto& self = this->derived();   
        auto info = this->get_segment_pos(ptr);
        
        auto seg_ac = self.get_segment_accessor(info.seg_id);
        
        seg_ac.do_for_all_blocks_in(info.idx_in_seg, size_in_bytes, mgbase::forward<Func>(func));
    }
    
    template <typename Func>
    typename mgbase::result_of<Func (block_accessor_type&)>::type
    do_for_block_at(void* const ptr, Func&& func)
    {
        auto& self = this->derived();
        auto info = this->get_segment_pos(ptr);
        
        auto seg_ac = self.get_segment_accessor(info.seg_id);
        
        return seg_ac.do_for_block_at(info.idx_in_seg, mgbase::forward<Func>(func));
    }
    
    template <typename Func>
    typename mgbase::result_of<Func (block_accessor_type&)>::type
    do_for_block_at(const abs_block_id_type& id, Func&& func)
    {
        auto& self = this->derived();
        
        auto seg_ac = self.get_segment_accessor(id.seg_id);
        auto pg_ac = seg_ac.get_page_accessor(id.pg_id);
        auto blk_ac = pg_ac.get_block_accessor(id.blk_id);
        
        return mgbase::forward<Func>(func)(blk_ac);
    }
    
    template <typename Conf>
    void make_segment(const segment_id_type seg_id, const Conf& conf)
    {
        auto& self = this->derived();
        auto& sh_sp = self.get_sharer_space();
        
        sh_sp.make_segment(seg_id, conf);
        
        segs_[seg_id] = mgbase::make_unique<segment_type>(self, conf);
    }
    
protected:
    segment_type& get_segment(const segment_id_type seg_id)
    {
        MGBASE_ASSERT(segs_[seg_id]);
        return *segs_[seg_id];
    }
    
private:
    struct segment_pos
    {
        segment_id_type     seg_id;
        mgbase::uintptr_t   idx_in_seg;
    };
    
    segment_pos get_segment_pos(void* const ptr) const
    {
        auto& self = this->derived();
        
        const auto iptr = reinterpret_cast<mgbase::uintptr_t>(ptr);
        const auto max_seg_size = self.get_max_segment_size();
        
        const segment_id_type seg_id =
            static_cast<segment_id_type>(iptr / max_seg_size);
        
        auto& seg = segs_[seg_id];
        
        const auto app_iptr =
            reinterpret_cast<mgbase::uintptr_t>(seg->get_app_ptr());
        
        return {
            seg_id
        ,   iptr - app_iptr
        };
    }
    
    mgbase::unique_ptr<segment_ptr_type []> segs_;
};

} // namespace mgdsm

