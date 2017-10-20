
#pragma once

#include <menps/mecom/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace mpi {

class mpi_endpoint
    : public mecom::endpoint
{
public:
    mpi_endpoint(int* const argc, char*** const argv);
    
    virtual ~mpi_endpoint();
};

mefdn::unique_ptr<mpi_endpoint> make_endpoint(int* argc, char*** argv);

} // namespace mpi
} // namespace mecom
} // namespace menps

