
#pragma once

#include "basic_sharer_segment_accessor.hpp"
#include "sharer_segment.hpp"

namespace mgdsm {

struct sharer_segment_accessor_policy
{
    typedef sharer_segment::accessor       derived_type;
    
    typedef page_id_t                   page_id_type;
    
    typedef mgbase::ptrdiff_t           difference_type;
    
    typedef manager_segment_proxy::acquire_read_result  acquire_read_result_type;
    typedef manager_segment_proxy::acquire_write_result acquire_write_result_type;
};

class sharer_segment::accessor
    : public basic_sharer_segment_accessor<sharer_segment_accessor_policy>
{
public:
    explicit accessor(sharer_segment& seg)
        : seg_(seg)
        , seg_lk_(seg.get_lock())
    { }
    
    inline sharer_page::accessor get_page_accessor(const page_id_t pg_id) MGBASE_NOEXCEPT;
    
private:
    friend class basic_sharer_segment_accessor<sharer_segment_accessor_policy>;
    
    sharer_segment_entry& get_segment_entry() const MGBASE_NOEXCEPT {
        return seg_;
    }
    manager_segment_proxy& get_manager() const MGBASE_NOEXCEPT {
        return seg_.get_manager();
    }
    
    sharer_segment&                     seg_;
    sharer_segment::unique_lock_type    seg_lk_;
};

sharer_segment::accessor sharer_segment::get_accessor() MGBASE_NOEXCEPT {
    return sharer_segment::accessor(*this);
}

} // namespace mgdsm

