
#pragma once

#include "device/mpi3/mpi3_interface.hpp"
#include "device/mpi3/rma/rma_window.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace mpi3 {

class commander
{
public:
    commander(endpoint&);
    ~commander();
    
    mpi3_interface& get_mpi_interface() noexcept;
    
    rma_window& get_win() noexcept;
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

mefdn::unique_ptr<commander> make_commander(endpoint&);

} // namespace mpi3
} // namespace mecom
} // namespace menps

