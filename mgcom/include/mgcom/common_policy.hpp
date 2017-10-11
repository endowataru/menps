
#pragma once

#include <mgcom/common.hpp>

namespace mgcom {

struct common_policy
{
    typedef mgcom::process_id_t     process_id_type;
    
    static process_id_t current_process_id() MGBASE_NOEXCEPT {
        return mgcom::current_process_id();
    }
    
    static mgbase::size_t number_of_processes() MGBASE_NOEXCEPT {
        return mgcom::number_of_processes();
    }
    
    bool valid_process_id(const process_id_t proc) MGBASE_NOEXCEPT {
        return proc < number_of_processes();
    }
};

} // namespace mgcom

