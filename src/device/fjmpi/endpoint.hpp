
#pragma once

#include "device/mpi/endpoint.hpp"

namespace mgcom {
namespace fjmpi {

class fjmpi_endpoint
    : public mpi::mpi_endpoint
{
public:
    fjmpi_endpoint(int* argc, char*** argv);
    
    ~fjmpi_endpoint();
};

mgbase::unique_ptr<fjmpi_endpoint> make_endpoint(int* argc, char*** argv);

} // namespace fjmpi
} // namespace mgcom

