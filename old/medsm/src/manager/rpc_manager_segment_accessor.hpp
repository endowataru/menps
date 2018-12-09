
#pragma once

#include "basic_rpc_manager_segment_accessor.hpp"
#include "rpc_manager_segment.hpp"
#include "rpc_manager_space.hpp"

namespace menps {
namespace medsm {

struct rpc_manager_segment_accessor_policy
{
    typedef rpc_manager_segment::accessor   derived_type;
    typedef page_id_t                       page_id_type;
    typedef mecom::process_id_t             process_id_type;
};

class rpc_manager_segment::accessor
    : public basic_rpc_manager_segment_accessor<rpc_manager_segment_accessor_policy>
{
public:
    explicit accessor(rpc_manager_space& sp, const segment_id_t seg_id)
        : sp_(sp)
        , seg_id_(seg_id)
        , seg_(sp.get_segment(seg_id))
    { }
    
    inline rpc_manager_page::accessor get_page_accessor(page_id_t pg_id);
    
private:
    friend class basic_rpc_manager_segment_accessor<rpc_manager_segment_accessor_policy>;
    
    rpc_manager_space& get_space() const noexcept {
        return sp_;
    }
    segment_id_t get_segment_id() const noexcept {
        return seg_id_;
    }
    rpc_manager_segment_shard& get_segment_shard() const noexcept {
        return seg_;
    }
    
    rpc_manager_space&      sp_;
    segment_id_t            seg_id_;
    rpc_manager_segment&    seg_;
};

rpc_manager_segment::accessor rpc_manager_space::get_segment_accessor(const segment_id_t seg_id) {
    return rpc_manager_segment::accessor(*this, seg_id);
}

} // namespace medsm
} // namespace menps

