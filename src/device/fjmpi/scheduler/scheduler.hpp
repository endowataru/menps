
#pragma once

#include "device/mpi/mpi_interface.hpp"
#include "device/fjmpi/fjmpi_interface.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace fjmpi {

class command_producer;

class scheduler
{
public:
    scheduler();
    ~scheduler();
    
    scheduler(const scheduler&) = delete;
    scheduler& operator = (const scheduler&) = delete;
    
    command_producer& get_command_producer() MGBASE_NOEXCEPT;
    
    mpi::mpi_interface& get_mpi_interface() MGBASE_NOEXCEPT;
    
    fjmpi_interface& get_fjmpi_interface() MGBASE_NOEXCEPT;
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

mgbase::unique_ptr<scheduler> make_scheduler();

} // namespace fjmpi
} // namespace mgcom

