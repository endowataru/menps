
#pragma once

#include <mgbase/utility/move.hpp>
#include <vector>

namespace mgdsm {

template <typename Traits>
class basic_sharer_space
{
    typedef typename Traits::derived_type       derived_type;
    
    typedef typename Traits::segment_type       segment_type;
    typedef typename Traits::segment_ptr_type   segment_ptr_type;
    typedef typename Traits::segment_id_type    segment_id_type;
    typedef typename Traits::segment_accessor_type segment_accessor_type;
    
    typedef typename Traits::lock_type          lock_type;
    
    // TODO : reduce dependency
    typedef typename Traits::manager_segment_ptr_type   manager_segment_ptr_type;

public:
    template <typename Conf>
    explicit basic_sharer_space(const Conf& conf)
        : segs_(conf.num_segments)
    { }
    
private:
    segment_type& get_segment(const segment_id_type seg_id)
    {
        auto& self = this->derived();
        auto& manager = self.get_manager();
        
        // Start the critical section.
        const auto lk = Traits::get_lock(lock_);
        
        auto& seg_ptr = segs_[seg_id];
        
        if (! seg_ptr)
        {
            // The segment is not cached in this process.
            
            // Load the segment from the manager.
            auto man_seg = manager.make_segment_proxy(seg_id);
            
            seg_ptr = self.create_sharer_segment(seg_id, mgbase::move(man_seg));
        }
        
        return *seg_ptr;
    }
    
public:
    segment_accessor_type get_segment_accessor(const segment_id_type seg_id)
    {
        auto& seg = this->get_segment(seg_id);
        
        return seg.get_accessor();
    }
    
private:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    
    lock_type                       lock_;
    std::vector<segment_ptr_type>   segs_;
};

} // namespace mgdsm

