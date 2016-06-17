
#include "barrier.impl.hpp"

namespace mgcom {
namespace mpi {
namespace collective {

namespace /*unnamed*/ {

barrier_impl g_impl;

} // unnamed namespace

void barrier()
{
    barrier_cb cb;
    barrier_impl::handlers<g_impl>::start(cb).wait();
}

namespace /*unnamed*/ {

class mpi_requester
    : public requester
{
public:
    mpi_requester()
    {
        g_impl.initialize();
    }
    
    virtual ~mpi_requester()
    {
        // do nothing
    }
    
    virtual void barrier() {
        mgcom::mpi::collective::barrier();
    }
    
    virtual void broadcast(const untyped::broadcast_params& params) {
        mgcom::mpi::collective::untyped::broadcast(params);
    }
    
    virtual void allgather(const untyped::allgather_params& params) {
        mgcom::mpi::collective::untyped::allgather(params);
    }
    
    virtual void alltoall(const untyped::alltoall_params& params) {
        mgcom::mpi::collective::untyped::alltoall(params);
    }
};

} // unnamed namespace

mgbase::unique_ptr<requester> make_requester()
{
    return mgbase::unique_ptr<requester>(new mpi_requester);
}

} // namespace collective
} // namespace mpi
} // namespace mgcom

