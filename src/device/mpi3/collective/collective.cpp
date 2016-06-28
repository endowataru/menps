
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
        mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
        
        while (MGBASE_UNLIKELY(
            !mi_.try_ibarrier({
                { comm_.get() }
            ,   mgbase::make_operation_store_release(&flag, true)
            })
        )) {
            mgbase::ult::this_thread::yield();
        }
        
        while (!flag.load(mgbase::memory_order_acquire)) {
            mgbase::ult::this_thread::yield();
        }
    }
    
    virtual void broadcast(const untyped::broadcast_params& params)
    {
        mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
        
        while (MGBASE_UNLIKELY(
            !mi_.try_ibcast({
                { params, comm_.get() }
            ,   mgbase::make_operation_store_release(&flag, true)
            })
        )) {
            mgbase::ult::this_thread::yield();
        }
        
        while (!flag.load(mgbase::memory_order_acquire)) {
            mgbase::ult::this_thread::yield();
        }
    }
    
    virtual void allgather(const untyped::allgather_params& params)
    {
        mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
        
        while (MGBASE_UNLIKELY(
            !mi_.try_iallgather({
                { params, comm_.get() }
            ,   mgbase::make_operation_store_release(&flag, true)
            })
        )) {
            mgbase::ult::this_thread::yield();
        }
        
        while (!flag.load(mgbase::memory_order_acquire)) {
            mgbase::ult::this_thread::yield();
        }
    }
    
    virtual void alltoall(const untyped::alltoall_params& params)
    {
        mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
        
        while (MGBASE_UNLIKELY(
            !mi_.try_ialltoall({
                { params, comm_.get() }
            ,   mgbase::make_operation_store_release(&flag, true)
            })
        )) {
            mgbase::ult::this_thread::yield();
        }
        
        while (!flag.load(mgbase::memory_order_acquire)) {
            mgbase::ult::this_thread::yield();
        }
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

