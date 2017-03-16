
#pragma once

#include <mgdsm/segment.hpp>

namespace mgdsm {

class dsm_segment
    : public segment
{
public:
    template <typename Conf>
    explicit dsm_segment(const Conf& conf)
        : protector_(&conf.protector)
        , seg_id_(conf.seg_id)
        , app_ptr_(conf.app_ptr)
        , size_(conf.page_size * conf.num_pages)
    {
        auto cconf = protector_space::proxy::create_conf_type();
        cconf.num_pages      = conf.num_pages;
        cconf.page_size      = conf.page_size;
        cconf.block_size     = conf.block_size;
        cconf.app_ptr        = conf.app_ptr;
        cconf.sys_ptr        = conf.sys_ptr;
        cconf.index_in_file  = conf.index_in_file;
        cconf.copy_data      = conf.copy_data;
        
        protector_->create_segment(seg_id_, cconf);
    }
    
    ~dsm_segment()
    {
        // manager_->destroy_segment();
    }
    
    virtual void* get_ptr() const MGBASE_NOEXCEPT MGBASE_OVERRIDE
    {
        return app_ptr_;
    }
    
    virtual mgbase::size_t get_size_in_bytes() const MGBASE_NOEXCEPT MGBASE_OVERRIDE {
        return size_;
    }
    
private:
    protector_space::proxy*     protector_;
    segment_id_t                seg_id_;
    void*                       app_ptr_;
    mgbase::size_t              size_;
};

} // namespace mgdsm

