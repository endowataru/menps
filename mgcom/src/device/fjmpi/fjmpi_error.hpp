
#pragma once

#include <mgcom.hpp>

namespace mgcom {
namespace fjmpi {

class fjmpi_error
{
public:
    template <typename T>
    static T assert_zero(const T result)
    {
        #ifdef MGBASE_DEBUG
        if (result != 0) {
            emit();
        }
        #endif
        return result;
    }
    
    template <typename T>
    static T assert_not_error(const T result)
    {
        #ifdef MGBASE_DEBUG
        if (result == FJMPI_RDMA_ERROR) {
            emit();
        }
        #endif
        return result;
    }
    
    MGBASE_NORETURN
    static void emit()
    {
        throw fjmpi_error();
    }
};

} // namespace fjmpi
} // namespace mgcom

