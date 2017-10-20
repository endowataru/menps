
#include "device/mpi3/mpi3_interface.hpp"
#include <menps/mecom/collective/requester.hpp>
#include "collective.hpp"
#include <menps/medev/mpi/communicator.hpp>
#include <menps/mecom/ult.hpp>

namespace menps {
namespace mecom {

namespace collective {
namespace /*unnamed*/ {

class mpi3_requester
    : public requester
{
public:
    explicit mpi3_requester(mpi3::mpi3_interface& mi)
        : mi_(mi)
        , comm_(
            medev::mpi::communicator::duplicate(
                mi
            ,   MPI_COMM_WORLD /*TODO*/
            ,   "MECOM_MPI3_COLLECTIVE"
            )
        )
    { }
    
    virtual void barrier() MEFDN_OVERRIDE
    {
        mi_.barrier({ comm_.get() });
    }
    
    virtual void broadcast(const untyped::broadcast_params& params) MEFDN_OVERRIDE
    {
        mi_.broadcast({
            static_cast<int>(params.root)
        ,   params.ptr
        ,   static_cast<int>(params.num_bytes)
        ,   comm_.get()
        });
    }
    
    virtual void allgather(const untyped::allgather_params& params) MEFDN_OVERRIDE
    {
        mi_.allgather({
            params.src
        ,   params.dest
        ,   static_cast<int>(params.num_bytes)
        ,   comm_.get()
        });
    }
    
    virtual void alltoall(const untyped::alltoall_params& params) MEFDN_OVERRIDE
    {
        mi_.alltoall({
            params.src
        ,   params.dest
        ,   static_cast<int>(params.num_bytes)
        ,   comm_.get()
        });
    }
    
private:
    mpi3::mpi3_interface&       mi_;
    medev::mpi::communicator    comm_;
};

} // unnamed namespace
} // namespace collective

namespace mpi3 {

mefdn::unique_ptr<mecom::collective::requester> make_collective_requester(mpi3_interface& mi)
{
    return mefdn::make_unique<collective::mpi3_requester>(mi);
}

} // namespace mpi3

} // namespace mecom
} // namespace menps

