
#include <menps/medev2/ucx/ucp/direct_facade.hpp>
#include <menps/medev2/ucx/ucp/ucp_policy.hpp>

int main()
{
    using namespace menps::medev2::ucx::ucp;
    
    using policy = ucp_policy<direct_facade_policy>;
    
    policy::ucp_facade_type uf;
    
    auto conf = policy::config_type::read(uf, nullptr, nullptr);
    
    ucp_params ctx_params = ucp_params();
    ctx_params.field_mask = UCP_PARAM_FIELD_FEATURES;
    ctx_params.features   = UCP_FEATURE_RMA;
    
    auto ctx = policy::context_type::init(uf, &ctx_params, conf.get());
    
    auto mem = policy::memory_type::map_allocate(uf, ctx.get(), 4096 /*TODO*/); 
    
    auto rkey_buf = policy::packed_rkey_type::pack(uf, ctx.get(), mem.get());
    
    ucp_worker_params_t wk_params = ucp_worker_params_t();
    auto wk = policy::worker_type::create(uf, ctx.get(), &wk_params);
    
    auto wk_addr = policy::worker_address_type::get_address(uf, wk.get());
    
    ucp_ep_params_t ep_params = ucp_ep_params_t();
    ep_params.field_mask = UCP_EP_PARAM_FIELD_REMOTE_ADDRESS;
    ep_params.address    = wk_addr.get();
    
    auto ep = policy::endpoint_type::create(uf, wk.get(), &ep_params);
    
    auto rkey = policy::remote_key_type::unpack(uf, ep.get(), rkey_buf.get());
    
    const auto rptr = static_cast<int*>(mem.get_address());
    const auto raddr = reinterpret_cast<menps::mefdn::uintptr_t>(rptr);
    
    fmt::print("{}\n", *rptr);
    
    int x = 123;
    
    ep.put(&x, sizeof(int), raddr, rkey.get());
    
    fmt::print("{}\n", *rptr);
    
    return 0;
}

