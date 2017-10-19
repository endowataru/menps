
#include <menps/medev/mpi/communicator.hpp>
#include <menps/medev/mpi/mpi_error.hpp>

namespace menps {
namespace medev {
namespace mpi {

class communicator::impl
{
public:
    /*implicit*/ impl(mpi1_requester& rqstr, const MPI_Comm comm)
        : rqstr_(rqstr)
        , comm_(comm)
    { }
    
    ~impl() /*noexcept*/
    {
        rqstr_.comm_free(&this->comm_);
    }
    
    mpi1_requester& rqstr_;
    MPI_Comm        comm_;
};

communicator::communicator(mpi1_requester& rqstr, const MPI_Comm comm)
    : impl_(mefdn::make_unique<impl>(rqstr, comm))
{ }

communicator::~communicator() /*noexcept*/ = default;

MPI_Comm communicator::get() const noexcept {
    return this->impl_->comm_;
}
std::string communicator::get_name() const noexcept {
    return get_comm_name(this->get());
}


communicator communicator::duplicate(
    mpi1_requester&     rqstr
,   const MPI_Comm      comm
,   const char* const   name
)
{
    const auto new_comm = rqstr.comm_dup(comm);
    
    rqstr.comm_set_name(new_comm, name);
    
    return communicator(rqstr, new_comm);
}

} // namespace mpi
} // namespace medev
} // namespace menps

