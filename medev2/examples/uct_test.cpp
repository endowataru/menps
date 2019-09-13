
#include <menps/medev2/ucx/uct/uct_policy.hpp>
#include <cmpth/sct/def_sct_itf.hpp>
#include <cmpth/ult_ext_itf.hpp>
#include <iostream>

int main()
{
    /*const char tl_name[] = "self";
    const char dev_name[] = "self";*/
    // TODO
    const char tl_name[] = "rc_mlx5";
    const char dev_name[] = "mlx5_0:1";
    
    using policy =
        menps::medev2::ucx::uct::direct_uct_itf<
            cmpth::ult_ext_itf<cmpth::def_sct_itf, cmpth::sync_tag_t::MCS>>;
    
    policy::uct_facade_type uf;
    
    auto async_ctx = policy::async_context_type::create(UCS_ASYNC_MODE_THREAD);
    auto wk = policy::worker_type::create(uf, async_ctx.get(), UCS_THREAD_MODE_MULTI);
    
    auto om_ret = policy::open_md(uf, tl_name, dev_name);
    const auto md_attr = om_ret.md.query();
    
    auto iface_conf =
        policy::iface_config_type::read(uf, om_ret.md.get(), tl_name, nullptr, nullptr);
    
    uct_iface_params_t iface_params = uct_iface_params_t();
    iface_params.open_mode = UCT_IFACE_OPEN_MODE_DEVICE;
    iface_params.mode.device.tl_name = tl_name;
    iface_params.mode.device.dev_name = dev_name;
    iface_params.stats_root = ucs_stats_get_root();
    iface_params.rx_headroom = 0;
    
    auto iface =
        policy::interface_type::open(uf, om_ret.md.get(), wk.get(),
            &iface_params, iface_conf.get());
    
    const auto iface_attr = iface.query();
    auto dev_addr = policy::get_device_address(iface, iface_attr);
    
    int x = 1234;
    
    auto mem = policy::memory_type::mem_reg(
        uf, om_ret.md.get(), &x, sizeof(x), UCT_MD_MEM_FLAG_FIXED | UCT_MD_MEM_ACCESS_ALL);
    
    auto packed_rkey = policy::pack_rkey(mem, md_attr);
    
    auto rkey = policy::remote_key_type::unpack(uf, om_ret.component, packed_rkey.get());
    
    #if 1
    auto ep = policy::endpoint_type::create(uf, iface.get());
    auto ep_addr = policy::get_ep_address(ep, iface_attr);
    
    ep.connect_to_ep(dev_addr.get(), ep_addr.get());
    
    int y = 123;
    
    const auto ret =
        ep.get_bcopy(
            reinterpret_cast<uct_unpack_callback_t>(&memcpy)
        ,   &y, sizeof(y)
        ,   reinterpret_cast<uintptr_t>(&x), rkey.get().rkey, nullptr
        );
    
    if (ret == UCS_INPROGRESS) {
        while (ep.flush(UCT_FLUSH_FLAG_LOCAL, nullptr) == UCS_INPROGRESS) {
            //wk.progress();
            iface.progress();
        }
    }
    
    std::cout << y << std::endl;
    
    #else
    auto iface_addr = policy::get_iface_address(iface, iface_attr);
    auto ep = policy::endpoint_type::create_connected(
        uf, iface.get(), dev_addr.get(), iface_addr.get());
    #endif
    
}

