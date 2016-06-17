
#include "direct_proxy.impl.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "completer.hpp"
#include "poll_thread.hpp"
#include "device/ibv/ibv.hpp"
#include "common/rma/region_allocator.hpp"
#include "device/ibv/rma/rma.hpp"

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

endpoint g_ep;
completer g_comp;

direct_proxy g_proxy(g_ep, g_comp);
poll_thread g_poll(g_ep.get_cq(), g_comp);

} // unnamed namespace

void initialize()
{
    g_ep.collective_initialize();
    
    rma::initialize_allocator();
    
    g_comp.initialize(); // completer requires allocator
    
    g_poll.start();
}

void finalize()
{
    g_poll.stop();
    
    g_comp.finalize();
     
    rma::finalize_allocator();
    
    g_ep.finalize();
}

} // namespace ibv
} // namespace mgcom

#include "ibv_interface.impl.hpp"

namespace mgcom {
namespace ibv {
namespace rma {
namespace untyped {

untyped::local_region register_region(const untyped::register_region_params& params)
{
    ibv_mr& mr = ibv::g_ep.register_memory(params.local_ptr, params.size_in_bytes);
    
    const region_key key = { params.local_ptr, mr.lkey };
    const local_region reg = { key, reinterpret_cast<mgbase::uint64_t>(&mr) };
    return reg;
}

void deregister_region(const untyped::deregister_region_params& params)
{
    ibv_mr* const mr = ibv::to_mr(params.region);
    ibv::g_ep.deregister_memory(*mr);
}

untyped::remote_region use_remote_region(const untyped::use_remote_region_params& params) 
{
    remote_region region;
    region.key = params.key;
    return region;
}

} // namespace untyped
} // namespace rma
} // namespace ibv
} // namespace mgcom

