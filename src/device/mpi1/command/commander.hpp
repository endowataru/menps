
#pragma once

#include "device/mpi/mpi_interface.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace mpi1 {

class commander
{
public:
    explicit commander(endpoint&);
    ~commander();
    
    mpi::mpi_interface& get_mpi_interface() MGBASE_NOEXCEPT;
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

mgbase::unique_ptr<commander> make_commander(endpoint&);

} // namespace mpi1
} // namespace mgcom

