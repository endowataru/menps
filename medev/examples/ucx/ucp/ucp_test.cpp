
#include <menps/medev/ucx/ucp/context.hpp>
#include <menps/medev/ucx/ucp/config.hpp>
#include <menps/medev/ucx/ucp/worker.hpp>
#include <menps/medev/ucx/ucp/endpoint.hpp>
#include <menps/medev/ucx/ucp/memory.hpp>
#include <menps/mefdn/external/fmt.hpp>

int main()
{
    using namespace menps::medev::ucx;
    namespace mefdn = menps::mefdn;
    
    ucp_params ctx_params = ucp_params();
    ctx_params.field_mask = UCP_PARAM_FIELD_FEATURES;
    ctx_params.features   = UCP_FEATURE_RMA;
    
    auto conf = ucp::read_config(nullptr, nullptr);
    
    auto ctx = ucp::init(&ctx_params, conf.get());
    
    conf.reset();
    
    ucp_mem_map_params mem_params = ucp_mem_map_params();
    mem_params.field_mask =
        UCP_MEM_MAP_PARAM_FIELD_ADDRESS |
        UCP_MEM_MAP_PARAM_FIELD_LENGTH |
        UCP_MEM_MAP_PARAM_FIELD_FLAGS;
    
    mem_params.length = 4096; // TODO
    mem_params.flags = UCP_MEM_MAP_ALLOCATE;
    
    auto mem = ucp::map_memory(ctx.get(), &mem_params);
    auto mem_attr = mem.query(UCP_MEM_ATTR_FIELD_ADDRESS);
    auto rkey_buf = mem.pack_rkey();
    
    ucp_worker_params_t wk_params = ucp_worker_params_t();
    auto wk = ucp::create_worker(ctx.get(), &wk_params);
    
    auto wk_addr = wk.get_address();
    
    ucp_ep_params_t ep_params = ucp_ep_params_t();
    ep_params.field_mask = UCP_EP_PARAM_FIELD_REMOTE_ADDRESS;
    ep_params.address    = wk_addr.get();
    
    auto ep = ucp::create_endpoint(wk.get(), &ep_params);
    
    auto rkey = rkey_buf.unpack(ep.get());
    
    int* const rptr = static_cast<int*>(mem_attr.address);
    const auto raddr = reinterpret_cast<mefdn::uintptr_t>(rptr);
    
    fmt::print("{}\n", *rptr);
    
    int x = 123;
    
    ep.put(&x, sizeof(int), raddr, rkey.get());
    
    fmt::print("{}\n", *rptr);
    
    return 0;
}

