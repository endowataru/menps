
#pragma once

#include "device/mpi/mpi_interface.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace mpi1 {

class commander
{
public:
    commander();
    ~commander();
    
    mpi::mpi_interface& get_mpi_interface() noexcept;
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

mefdn::unique_ptr<commander> make_commander();

} // namespace mpi1
} // namespace mecom
} // namespace menps

