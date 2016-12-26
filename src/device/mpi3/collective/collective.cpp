
#include "device/mpi3/mpi3_interface.hpp"
#include <mgcom/collective/requester.hpp>
#include "collective.hpp"
#include "device/mpi/mpi_communicator.hpp"

namespace mgcom {

namespace collective {
namespace /*unnamed*/ {

class mpi3_requester
    : public requester
{
public:
    explicit mpi3_requester(mpi3::mpi3_interface& mi)
        : mi_(mi)
        , comm_(mi, MPI_COMM_WORLD /*TODO*/, "MGCOM_MPI3_COLLECTIVE") { }
    
    virtual void barrier()
    {
        ult::sync_flag flag;
        
        while (MGBASE_UNLIKELY(
            !mi_.try_ibarrier({
                { comm_.get() }
            ,   mgbase::make_callback_notify(&flag)
            })
        )) {
            ult::this_thread::yield();
        }
        
        flag.wait();
    }
    
    virtual void broadcast(const untyped::broadcast_params& params)
    {
        ult::sync_flag flag;
        
        while (MGBASE_UNLIKELY(
            !mi_.try_ibcast({
                { params, comm_.get() }
            ,   mgbase::make_callback_notify(&flag)
            })
        )) {
            ult::this_thread::yield();
        }
        
        flag.wait();
    }
    
    virtual void allgather(const untyped::allgather_params& params)
    {
        ult::sync_flag flag;
        
        while (MGBASE_UNLIKELY(
            !mi_.try_iallgather({
                { params, comm_.get() }
            ,   mgbase::make_callback_notify(&flag)
            })
        )) {
            ult::this_thread::yield();
        }
        
        flag.wait();
    }
    
    virtual void alltoall(const untyped::alltoall_params& params)
    {
        ult::sync_flag flag;
        
        while (MGBASE_UNLIKELY(
            !mi_.try_ialltoall({
                { params, comm_.get() }
            ,   mgbase::make_callback_notify(&flag)
            })
        )) {
            ult::this_thread::yield();
        }
        
        flag.wait();
    }
    
private:
    mpi3::mpi3_interface& mi_;
    mpi::mpi_communicator comm_;
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

