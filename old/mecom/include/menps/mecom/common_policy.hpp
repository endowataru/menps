
#pragma once

#include <menps/mecom/common.hpp>

namespace menps {
namespace mecom {

struct common_policy
{
    typedef mecom::process_id_t     process_id_type;
    
    static process_id_t current_process_id() noexcept {
        return mecom::current_process_id();
    }
    
    static mefdn::size_t number_of_processes() noexcept {
        return mecom::number_of_processes();
    }
    
    bool valid_process_id(const process_id_t proc) noexcept {
        return proc < number_of_processes();
    }
};

} // namespace mecom
} // namespace menps

