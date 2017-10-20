
#include "barrier.impl.hpp"

namespace menps {
namespace mecom {
namespace mpi {
namespace collective {

namespace /*unnamed*/ {

barrier_impl g_impl;

} // unnamed namespace

namespace /*unnamed*/ {

class mpi_requester
    : public requester
{
public:
    mpi_requester(mpi_interface& mi)
        : mi_(mi)
    {
        g_impl.initialize(mi);
    }
    
    virtual ~mpi_requester()
    {
        // do nothing
    }
    
    virtual void barrier() MEFDN_OVERRIDE
    {
        g_impl.barrier();
        #if 0
        barrier_cb cb;
        barrier_impl::handlers<g_impl>::start(cb).wait();
        #endif
    }
    
    virtual void broadcast(const untyped::broadcast_params& params) MEFDN_OVERRIDE
    {
        // do barrier to avoid deadlocking
        barrier();
        
        mi_.broadcast({
            process_id_to_rank(params.root)
        ,   params.ptr
        ,   static_cast<int>(params.num_bytes)
        ,   g_impl.get_communicator()
        });
    }
    
    virtual void allgather(const untyped::allgather_params& params) MEFDN_OVERRIDE
    {
        // do barrier to avoid deadlocking
        barrier();
        
        mi_.allgather({
            params.src
        ,   params.dest
        ,   static_cast<int>(params.num_bytes)
        ,   g_impl.get_communicator()
        });
    }
    
    virtual void alltoall(const untyped::alltoall_params& params) MEFDN_OVERRIDE
    {
        // do barrier to avoid deadlocking
        barrier();
        
        mi_.alltoall({
            params.src
        ,   params.dest
        ,   static_cast<int>(params.num_bytes)
        ,   g_impl.get_communicator()
        });
    }

private:
    mpi_interface& mi_;
};

} // unnamed namespace

mefdn::unique_ptr<requester> make_requester(mpi_interface& mi)
{
    return mefdn::make_unique<mpi_requester>(mi);
}

} // namespace collective
} // namespace mpi
} // namespace mecom
} // namespace menps

