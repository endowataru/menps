
#include "direct_proxy.impl.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "completer.hpp"
#include "poll_thread.hpp"
#include "device/ibv/ibv.hpp"

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
    g_comp.initialize();
    g_ep.collective_initialize();
    g_poll.start();
}

void finalize()
{
    g_poll.stop();
    g_ep.finalize();
    g_comp.finalize();
}

} // namespace ibv
} // namespace mgcom

#include "ibv_interface.impl.hpp"

namespace mgcom {
namespace rma {
namespace untyped {

local_region register_region(
    void* const     buf
,   const index_t   size_in_bytes
) {
    ibv_mr& mr = ibv::g_ep.register_memory(buf, size_in_bytes);
    
    const region_key key = { buf, mr.lkey };
    const local_region reg = { key, reinterpret_cast<mgbase::uint64_t>(&mr) };
    return reg;
}

void deregister_region(const local_region& region)
{
    ibv_mr* const mr = ibv::to_mr(region);
    ibv::g_ep.deregister_memory(*mr);
}

remote_region use_remote_region(
    const process_id_t  //proc_id
,   const region_key&   key
) {
    remote_region region;
    region.key = key;
    return region;
}

} // namespace untyped
} // namespace rma
} // namespace mgcom

