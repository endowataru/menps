
#pragma once

#include <mgth/common.hpp>
#include "global_ult_ref.hpp"

#include <mgdsm/space_ref.hpp>

#include <mgbase/unique_ptr.hpp>
#include <mgbase/memory/allocatable.hpp>

namespace mgth {

class global_ult_desc_pool
{
public:
    static const mgbase::size_t stack_size = 64 << 10; // TODO: adjustable
    
    struct config
    {
        void* stack_segment_ptr;
    };
    
    explicit global_ult_desc_pool(const config&);
    
    ~global_ult_desc_pool();
    
    global_ult_ref allocate_ult();
    
    void deallocate_ult(global_ult_ref&& th);
    
    global_ult_ref get_ult_ref_from_id(const ult_id& id);
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mgth

