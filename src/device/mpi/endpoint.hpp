
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace mpi {

class mpi_endpoint
    : public mgcom::endpoint
{
public:
    mpi_endpoint(int* const argc, char*** const argv);
    
    virtual ~mpi_endpoint();
};

mgbase::unique_ptr<mpi_endpoint> make_endpoint(int* argc, char*** argv);

} // namespace mpi
} // namespace mgcom

