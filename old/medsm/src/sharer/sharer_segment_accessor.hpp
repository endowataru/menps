
#pragma once

#include "basic_sharer_segment_accessor.hpp"
#include "sharer_segment.hpp"
#include "sharer_space.hpp"

namespace menps {
namespace medsm {

struct sharer_segment_accessor_policy
{
    typedef sharer_segment::accessor       derived_type;
    
    typedef page_id_t                   page_id_type;
    
    typedef mefdn::ptrdiff_t           difference_type;
    
    typedef manager_segment_proxy::acquire_read_result  acquire_read_result_type;
    typedef manager_segment_proxy::acquire_write_result acquire_write_result_type;
    typedef manager_segment_proxy::release_write_result release_write_result_type;
    
    typedef mecom::rma::paired_local_ptr<void>  plptr_type;
};

class sharer_segment::accessor
    : public basic_sharer_segment_accessor<sharer_segment_accessor_policy>
{
public:
    explicit accessor(sharer_segment& seg, const segment_id_t seg_id)
        : seg_(seg)
        , seg_id_(seg_id)
        // TODO: Segments are not locked.
        //       Although this doesn't cause problems in the current implementation,
        //       it disallows modifications on segments.
        //, seg_lk_(seg.get_lock())
    { }
    
    inline sharer_page::accessor get_page_accessor(const page_id_t pg_id) noexcept;
    
    segment_id_t get_segment_id() const noexcept {
        return this->seg_id_;
    }
    
    std::string to_string() const
    {
        return fmt::format(
            "seg_id:{}"
        ,   seg_id_
        );
    }
    
private:
    friend class basic_sharer_segment_accessor<sharer_segment_accessor_policy>;
    
    sharer_segment_entry& get_segment_entry() const noexcept {
        return seg_;
    }
    manager_segment_proxy& get_manager() const noexcept {
        return seg_.get_manager();
    }
    
    sharer_segment&                     seg_;
    const segment_id_t                  seg_id_;
    //sharer_segment::unique_lock_type    seg_lk_;
};

sharer_segment::accessor sharer_space::get_segment_accessor(const segment_id_t seg_id)
{
    auto& seg = this->get_segment(seg_id);
    
    return sharer_segment::accessor(seg, seg_id);
}

} // namespace medsm
} // namespace menps

