
#include "barrier.impl.hpp"

namespace mgcom {
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
    
    virtual void barrier() MGBASE_OVERRIDE
    {
        barrier_cb cb;
        barrier_impl::handlers<g_impl>::start(cb).wait();
    }
    
    virtual void broadcast(const untyped::broadcast_params& params) MGBASE_OVERRIDE
    {
        // do barrier to avoid deadlocking
        barrier();
        
        mi_.native_broadcast({
            params
        ,   g_impl.get_communicator()
        });
    }
    
    virtual void allgather(const untyped::allgather_params& params) MGBASE_OVERRIDE
    {
        // do barrier to avoid deadlocking
        barrier();
        
        mi_.native_allgather({
            params
        ,   g_impl.get_communicator()
        });
    }
    
    virtual void alltoall(const untyped::alltoall_params& params) MGBASE_OVERRIDE
    {
        // do barrier to avoid deadlocking
        barrier();
        
        mi_.native_alltoall({
            params
        ,   g_impl.get_communicator()
        });
    }

private:
    mpi_interface& mi_;
};

} // unnamed namespace

mgbase::unique_ptr<requester> make_requester(mpi_interface& mi)
{
    return mgbase::make_unique<mpi_requester>(mi);
}

} // namespace collective
} // namespace mpi
} // namespace mgcom

