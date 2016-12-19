
#pragma once

#include "basic_rpc_manager_page_entry.hpp"
#include "mgdsm_common.hpp"
#include <mgbase/threading/mutex.hpp>
#include <mgbase/threading/condition_variable.hpp>
#include <mgcom/rma/paired_local_ptr.hpp>
#include "process_id_set.hpp"
#include "rpc_manager_page_invalidator.hpp"

namespace mgdsm {

class rpc_manager_page_entry;

struct rpc_manager_page_entry_traits
{
    typedef rpc_manager_page_entry                      derived_type;
    
    typedef mgcom::rma::paired_local_ptr<void>          owner_plptr_type;
    
    typedef mgcom::process_id_t                         process_id_type;
    typedef process_id_set                              process_id_set_type;
    
    typedef rpc_manager_page_invalidator                invalidator_type;
    
    typedef mgbase::mutex                               mutex_type;
    typedef mgbase::condition_variable                  cv_type;
    typedef mgbase::unique_lock<mutex_type>             unique_lock_type;
    
    static unique_lock_type get_lock(mutex_type& mtx) {
        return unique_lock_type(mtx);
    }
    
    static bool is_invalid_plptr(const owner_plptr_type& plptr) MGBASE_NOEXCEPT {
        return plptr.ptr != MGBASE_NULLPTR;
    }
};

class rpc_manager_page_entry
    : public basic_rpc_manager_page_entry<rpc_manager_page_entry_traits>
{ };

} // namespace mgdsm

