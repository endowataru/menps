
#pragma once

#include "device/mpi/mpi_base.hpp"
#include "common/starter.hpp"
#include "mpi-ext.h"

namespace menps {
namespace mecom {
namespace fjmpi {

struct constants
{
    static const int max_nic_count = 4;
    static const int max_memid_count = 510;
    static const int max_tag_count = 15;
};

} // namespace fjmpi
} // namespace mecom
} // namespace menps

