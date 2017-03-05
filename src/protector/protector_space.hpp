
#pragma once

#include "basic_protector_space.hpp"
#include "protector_segment_accessor.hpp"
#include "protector_block_accessor.hpp"
#include "sharer/segment_locator.hpp"
#include "protector_segment.hpp"
#include <mgbase/shared_ptr.hpp>

namespace mgdsm {

class protector_space;

struct protector_space_policy
{
    typedef protector_space             derived_type;
    typedef segment_locator             interface_type;
    
    typedef protector_segment           segment_type;
    
    typedef protector_block_accessor    block_accessor_type;
    typedef segment_id_t                segment_id_type;
    typedef abs_block_id                abs_block_id_type;
};

class access_history;

class protector_space
    : public basic_protector_space<protector_space_policy>
{
    typedef basic_protector_space<protector_space_policy>   base;
    
public:
    template <typename Conf>
    explicit protector_space(const Conf& conf)
        : base(conf)
        , sh_sp_(conf.sharer_sp)
        , max_seg_size_(conf.max_seg_size)
        , fd_(conf.fd)
        , hist_(MGBASE_NULLPTR)
    { }
    
    protector_segment_accessor get_segment_accessor(const segment_id_t seg_id)
    {
        return protector_segment_accessor(
            this->get_segment(seg_id)
        ,   sh_sp_.get_segment_accessor(seg_id)
        );
    }
    
    class proxy;
    
    inline proxy make_proxy_collective();
    
    mgbase::size_t get_max_segment_size() const MGBASE_NOEXCEPT {
        return max_seg_size_;
    }
    int get_fd() const MGBASE_NOEXCEPT {
        return fd_;
    }
    
    void set_history(access_history& hist) {
        hist_ = &hist;
    }
    access_history& get_history() const MGBASE_NOEXCEPT {
        return *hist_;
    }
    
private:
    friend class basic_protector_space<protector_space_policy>;
    // friend base;
    
    sharer_space& get_sharer_space() const MGBASE_NOEXCEPT {
        return sh_sp_;
    }
    
    sharer_space&           sh_sp_;
    const mgbase::size_t    max_seg_size_;
    const int               fd_;
    access_history*         hist_;
};

mgbase::size_t protector_block_accessor::get_max_seg_size() const MGBASE_NOEXCEPT
{
    auto& sp = seg_.get_space();
    return sp.get_max_segment_size();
}


template <typename Conf>
protector_segment::protector_segment(protector_space& sp, const Conf& conf)
    : sp_(sp)
{   
    const auto size_in_bytes = conf.num_pages * conf.page_size;
    
    app_map_ = mgbase::mapped_memory::map(
        conf.app_ptr
    ,   size_in_bytes
    ,   PROT_NONE//PROT_READ | PROT_WRITE
    ,   MAP_FIXED | MAP_SHARED
    ,   sp_.get_fd()
    ,   conf.index_in_file
    );
    
    sys_map_ = mgbase::mapped_memory::map(
        conf.sys_ptr
    ,   size_in_bytes
    ,   PROT_READ | PROT_WRITE
    ,   MAP_FIXED | MAP_SHARED
    ,   sp_.get_fd()
    ,   conf.index_in_file
    );
}

} // namespace mgdsm

