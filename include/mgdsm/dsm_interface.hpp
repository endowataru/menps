
#pragma once

#include <mgbase/lang.hpp>

namespace mgdsm {

class dsm_interface
{
protected:
    dsm_interface() MGBASE_DEFAULT_NOEXCEPT = default;
    
public:
    virtual ~dsm_interface() = default;
    
    struct segment_info
    {
        segment_id_t    id;
        void*           ptr;
    };
    
    virtual segment_info create_segment(
        mgbase::size_t  size_in_bytes
    ,   mgbase::size_t  page_size_in_bytes
    ) = 0;
    
    virtual void destroy_segment(
        segment_id_t    segment_id
    ) = 0;
    
    #if 0
    virtual void* allocate(
        segment_id_t    segment_id
    ,   mgbase::size_t  alignment
    ,   mgbase::size_t  size_in_bytes
    ) = 0;
    
    virtual void deallocate(
        segment_id_t    segment_id
    ,   void*           ptr
    ) = 0;
    #endif
    
    virtual void pin(
        void*           ptr
    ,   mgbase::size_t  size_in_bytes
    ) = 0;
    
    virtual void unpin(
        void*           ptr
    ,   mgbase::size_t  size_in_bytes
    ) = 0;
    
    virtual void reconcile_all() = 0;
    
    virtual void flush_all() = 0;
    
    virtual void enable_on_this_thread() = 0;
    
    virtual void disable_on_this_thread() = 0;
};

} // namespace mgdsm

