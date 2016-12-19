
#pragma once

#include <mgdsm/segment.hpp>

namespace mgdsm {

class dsm_segment
    : public segment
{
public:
    template <typename Conf>
    explicit dsm_segment(const Conf& conf)
        : manager_(&conf.manager)
        , seg_id_(conf.seg_id)
    {
        manager_->create_segment(seg_id_, { conf.num_pages, conf.page_size });
    }
    
    ~dsm_segment()
    {
        // manager_->destroy_segment();
    }
    
private:
    rpc_manager_space::proxy*   manager_;
    segment_id_t                seg_id_;
};

} // namespace mgdsm

