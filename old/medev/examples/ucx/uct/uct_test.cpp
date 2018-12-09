
#include <menps/medev/ucx/ucs/async_context.hpp>
#include <menps/medev/ucx/uct/worker.hpp>
#include <menps/medev/ucx/uct/md_resource_list.hpp>
#include <menps/medev/ucx/uct/tl_resource_list.hpp>
#include <menps/medev/ucx/uct/memory_domain.hpp>
#include <menps/medev/ucx/uct/iface_config.hpp>
#include <menps/medev/ucx/uct/interface.hpp>
#include <menps/medev/ucx/uct/allocated_memory.hpp>
#include <menps/medev/ucx/uct/endpoint.hpp>
#include <cstring>
#include <menps/mefdn/external/fmt.hpp>

using namespace menps::medev::ucx;

uct::memory_domain open_md(const char* tl_name, const char* dev_name)
{
    auto md_ress = uct::query_md_resources();
    
    MEFDN_RANGE_BASED_FOR(auto&& md_res, md_ress) {
        auto md = uct::open_memory_domain(&md_res);
        auto tl_ress = uct::query_tl_resources(md.get());
        
        MEFDN_RANGE_BASED_FOR(auto&& tl_res, tl_ress) {
            if ((strcmp(tl_res.tl_name, tl_name) == 0)
                && (strcmp(tl_res.dev_name, dev_name) == 0))
            {
                return md;
            }
        }
    }
    
    return {};
}

ucs_status_t am_handler_func(
    void* const         arg
,   void* const         data
,   const size_t        length
,   const unsigned int  flags
) {
    fmt::print("handler\n");
    
    return UCS_OK;
}

int main()
{
    const char tl_name[] = "self";
    const char dev_name[] = "self";
    
    auto async_ctx = ucs::create_async_context(UCS_ASYNC_MODE_THREAD);
    auto wk = uct::create_worker(async_ctx.get(), UCS_THREAD_MODE_MULTI);
    
    auto md = open_md(tl_name, dev_name);
    
    auto iface_conf = uct::read_iface_config(md.get(), tl_name, nullptr, nullptr);
    
    uct_iface_params_t iface_params = uct_iface_params_t();
    iface_params.open_mode = UCT_IFACE_OPEN_MODE_DEVICE;
    iface_params.mode.device.tl_name = tl_name;
    iface_params.mode.device.dev_name = dev_name;
    iface_params.stats_root = ucs_stats_get_root();
    iface_params.rx_headroom = 0;
    
    auto iface = uct::open_interface(md.get(), wk.get(), &iface_params, iface_conf.get());
    
    iface_conf.reset();
    
    const uct::am_id_t am_id = 1;
    
    iface.set_am_handler(am_id, &am_handler_func, nullptr, UCT_CB_FLAG_ASYNC);
    
    auto iface_addr = iface.get_address();
    auto dev_addr = iface.get_device_address();
    
    auto ep = uct::create_endpoint_connected(iface.get(), dev_addr.get(), iface_addr.get());
    
    //auto ep = uct::create_endpoint(iface.get());
    
    
    //auto md = uct::open_memory_domain(/*name*/, md_conf);
    uct_ep_am_short(ep.get(), am_id, 0, nullptr, 0);
    
    return 0;
    
}

