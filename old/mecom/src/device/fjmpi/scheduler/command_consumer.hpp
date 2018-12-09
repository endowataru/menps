
#pragma once

#include "command_queue.hpp"
#include "device/mpi/command/mpi_completer.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

class command_consumer
    : protected virtual command_queue
{
public:
    command_consumer(endpoint&);
    
    virtual ~command_consumer();
    
    command_consumer(const command_consumer&) = delete;
    command_consumer& operator = (const command_consumer&) = delete;
    
protected:
    mpi::mpi_completer& get_mpi_completer() noexcept;
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace fjmpi
} // namespace mecom
} // namespace menps

