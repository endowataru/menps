
#pragma once

#include "basic_rpc_manager_page_entry.hpp"
#include "medsm_common.hpp"
#include <menps/mecom/rma/paired_local_ptr.hpp>
#include "process_id_set.hpp"
#include "rpc_manager_page_invalidator.hpp"
#include <menps/mecom/common_policy.hpp>
#include <menps/medsm/ult.hpp>

namespace menps {
namespace medsm {

class rpc_manager_page_entry;

struct rpc_manager_page_entry_traits
    : mecom::common_policy
{
    typedef rpc_manager_page_entry                      derived_type;
    
    typedef mecom::rma::paired_local_ptr<void>          owner_plptr_type;
    
    typedef process_id_set                              process_id_set_type;
    
    typedef rpc_manager_page_invalidator                invalidator_type;
    
    typedef ult::mutex                                  mutex_type;
    typedef ult::condition_variable                     cv_type;
    typedef ult::unique_lock<mutex_type>                unique_lock_type;
    
    static unique_lock_type get_lock(mutex_type& mtx) {
        return unique_lock_type(mtx);
    }
    
    static bool is_invalid_plptr(const owner_plptr_type& plptr) noexcept {
        return plptr.ptr == nullptr;
    }
    
    static owner_plptr_type make_invalid_plptr() noexcept {
        return owner_plptr_type();
    }
};

class rpc_manager_page_entry
    : public basic_rpc_manager_page_entry<rpc_manager_page_entry_traits>
{ };

} // namespace medsm
} // namespace menps

