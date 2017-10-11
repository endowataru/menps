
#include <mgdev/mpi/communicator.hpp>
#include <mgdev/mpi/mpi_error.hpp>

namespace mgdev {
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
    : impl_(mgbase::make_unique<impl>(rqstr, comm))
{ }

communicator::~communicator() /*noexcept*/ = default;

MPI_Comm communicator::get() const MGBASE_NOEXCEPT {
    return this->impl_->comm_;
}
std::string communicator::get_name() const MGBASE_NOEXCEPT {
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
} // namespace mgdev

