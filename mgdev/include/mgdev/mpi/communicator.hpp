
#pragma once

#include <mgdev/mpi/mpi1_requester.hpp>
#include <mgbase/unique_ptr.hpp>
#include <string>

namespace mgdev {
namespace mpi {

class communicator
{
public:
    explicit communicator(mpi1_requester&, MPI_Comm);
    
    ~communicator() /*noexcept*/;
    
    communicator(communicator&&) MGBASE_NOEXCEPT;
    communicator& operator = (communicator&&) MGBASE_NOEXCEPT;
    
    MPI_Comm get() const MGBASE_NOEXCEPT;
    
    std::string get_name() const MGBASE_NOEXCEPT;
    
    static communicator duplicate(mpi1_requester&, MPI_Comm, const char*);
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mpi
} // namespace mgdev

