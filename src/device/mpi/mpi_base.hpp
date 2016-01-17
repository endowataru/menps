
#pragma once

#include <mgcom.hpp>
#include <mpi.h>
#include <mgbase/threading/spinlock.hpp>

#include "mpi_error.hpp"

namespace mgcom {
namespace mpi_base {

void initialize(int* argc, char*** argv);

void finalize();

typedef mgbase::spinlock  lock_type;

inline lock_type& get_lock() MGBASE_NOEXCEPT
{
    static lock_type lc;
    return lc;
}

}
}

