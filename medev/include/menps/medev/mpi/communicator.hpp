
#pragma once

#include <menps/medev/mpi/mpi1_requester.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <string>

namespace menps {
namespace medev {
namespace mpi {

class communicator
{
public:
    explicit communicator(mpi1_requester&, MPI_Comm);
    
    ~communicator() /*noexcept*/;
    
    communicator(communicator&&) noexcept;
    communicator& operator = (communicator&&) noexcept;
    
    MPI_Comm get() const noexcept;
    
    std::string get_name() const noexcept;
    
    static communicator duplicate(mpi1_requester&, MPI_Comm, const char*);
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace mpi
} // namespace medev
} // namespace menps

