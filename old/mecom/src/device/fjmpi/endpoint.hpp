
#pragma once

#include "device/mpi/endpoint.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

class fjmpi_endpoint
    : public mpi::mpi_endpoint
{
public:
    fjmpi_endpoint(int* argc, char*** argv);
    
    ~fjmpi_endpoint();
};

mefdn::unique_ptr<fjmpi_endpoint> make_endpoint(int* argc, char*** argv);

} // namespace fjmpi
} // namespace mecom
} // namespace menps

