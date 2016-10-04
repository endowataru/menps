
#pragma once

#include "global_ult_ref.hpp"
#include <mgbase/unique_ptr.hpp>

#include <mgdsm/dsm_interface.hpp>

namespace mgth {

class global_ult_desc_pool
{
public:
    explicit global_ult_desc_pool(mgdsm::dsm_interface& dsm);
    
    ~global_ult_desc_pool();
    
    global_ult_ref allocate_ult();
    
    void deallocate_ult(global_ult_ref&& th);
    
    global_ult_ref get_ult_ref_from_id(const ult_id& id);
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mgth

