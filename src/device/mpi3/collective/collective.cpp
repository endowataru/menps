
#include "device/mpi3/mpi3_interface.hpp"
#include <mgcom/collective/requester.hpp>
#include "collective.hpp"
#include <mgdev/mpi/communicator.hpp>
#include <mgcom/ult.hpp>

namespace mgcom {

namespace collective {
namespace /*unnamed*/ {

class mpi3_requester
    : public requester
{
public:
    explicit mpi3_requester(mpi3::mpi3_interface& mi)
        : mi_(mi)
        , comm_(
            mgdev::mpi::communicator::duplicate(
                mi
            ,   MPI_COMM_WORLD /*TODO*/
            ,   "MGCOM_MPI3_COLLECTIVE"
            )
        )
    { }
    
    virtual void barrier() MGBASE_OVERRIDE
    {
        mi_.barrier({ comm_.get() });
    }
    
    virtual void broadcast(const untyped::broadcast_params& params) MGBASE_OVERRIDE
    {
        mi_.broadcast({
            static_cast<int>(params.root)
        ,   params.ptr
        ,   static_cast<int>(params.num_bytes)
        ,   comm_.get()
        });
    }
    
    virtual void allgather(const untyped::allgather_params& params) MGBASE_OVERRIDE
    {
        mi_.allgather({
            params.src
        ,   params.dest
        ,   static_cast<int>(params.num_bytes)
        ,   comm_.get()
        });
    }
    
    virtual void alltoall(const untyped::alltoall_params& params) MGBASE_OVERRIDE
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
    mgdev::mpi::communicator    comm_;
};

} // unnamed namespace
} // namespace collective

namespace mpi3 {

mgbase::unique_ptr<mgcom::collective::requester> make_collective_requester(mpi3_interface& mi)
{
    return mgbase::make_unique<collective::mpi3_requester>(mi);
}

} // namespace mpi3

} // namespace mgcom

