
#pragma once

#include "device/mpi3/mpi3_interface.hpp"
#include "device/mpi3/rma/rma_window.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace mpi3 {

class commander
{
public:
    commander(endpoint&);
    ~commander();
    
    mpi3_interface& get_mpi_interface() MGBASE_NOEXCEPT;
    
    rma_window& get_win() MGBASE_NOEXCEPT;
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

mgbase::unique_ptr<commander> make_commander(endpoint&);

} // namespace mpi3
} // namespace mgcom

