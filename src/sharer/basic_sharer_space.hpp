
#pragma once

#include <mgbase/utility/move.hpp>
#include <vector>

namespace mgdsm {

template <typename Traits>
class basic_sharer_space
{
    typedef typename Traits::derived_type           derived_type;
    
    typedef typename Traits::segment_type           segment_type;
    typedef typename Traits::segment_ptr_type       segment_ptr_type;
    typedef typename Traits::segment_id_type        segment_id_type;
    typedef typename Traits::segment_accessor_type  segment_accessor_type;
    typedef typename Traits::segment_conf_type      segment_conf_type;
    
    typedef typename Traits::lock_type              lock_type;
    
    // TODO : reduce dependency
    typedef typename Traits::manager_segment_ptr_type   manager_segment_ptr_type;

public:
    template <typename Conf>
    explicit basic_sharer_space(const Conf& conf)
        : segs_(mgbase::make_unique<segment_ptr_type []>(conf.num_segments))
    { }
    
protected:
    segment_type& get_segment(const segment_id_type seg_id)
    {
        // Start the critical section.
        const auto lk = Traits::get_lock(lock_);
        
        auto& seg_ptr = segs_[seg_id];
        MGBASE_ASSERT(seg_ptr);
        
        return *seg_ptr;
    }
    
private:
    // Note: Old GCC cannot use internal linkage class for template argument
    struct sharer_segment_conf {
        manager_segment_proxy_ptr   manager;
        void*                       sys_ptr;
    };
    
public:
    template <typename Conf>
    void make_segment(const segment_id_type seg_id, const Conf& conf)
    {
        auto& self = this->derived();
        
        // Start the critical section.
        const auto lk = Traits::get_lock(lock_);
        
        auto& manager = self.get_manager();
        
        segment_conf_type seg_conf{};
        seg_conf.num_pages  = conf.num_pages;
        seg_conf.page_size  = conf.page_size;
        seg_conf.block_size = conf.block_size;
        
        manager.make_segment(seg_id, seg_conf);
        
        MGBASE_ASSERT(segs_[seg_id] == MGBASE_NULLPTR);
        
        // Load the segment from the manager.
        sharer_segment_conf sh_seg_conf{
            manager.make_segment_proxy(seg_id)
        ,   conf.sys_ptr
        };
        
        segs_[seg_id] = mgbase::make_unique<segment_type>(sh_seg_conf);
    }
    
    #if 0
    
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
    #endif
    
private:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    
    lock_type                                   lock_;
    mgbase::unique_ptr<segment_ptr_type []>     segs_;
};

} // namespace mgdsm

