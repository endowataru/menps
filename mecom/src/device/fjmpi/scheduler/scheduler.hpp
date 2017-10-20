
#pragma once

#include "device/mpi/mpi_interface.hpp"
#include "device/fjmpi/fjmpi_interface.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace fjmpi {

class command_producer;

class scheduler
{
public:
    explicit scheduler(endpoint&);
    ~scheduler();
    
    scheduler(const scheduler&) = delete;
    scheduler& operator = (const scheduler&) = delete;
    
    command_producer& get_command_producer() noexcept;
    
    mpi::mpi_interface& get_mpi_interface() noexcept;
    
    fjmpi_interface& get_fjmpi_interface() noexcept;
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

mefdn::unique_ptr<scheduler> make_scheduler(endpoint&);

} // namespace fjmpi
} // namespace mecom
} // namespace menps

