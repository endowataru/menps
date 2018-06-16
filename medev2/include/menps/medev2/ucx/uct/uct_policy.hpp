
#pragma once

#include <menps/medev2/ucx/uct/direct_facade.hpp>
#include <menps/medev2/ucx/ucs/async_context.hpp>
#include <menps/medev2/ucx/uct/worker.hpp>
#include <menps/medev2/ucx/uct/md_resource_list.hpp>
#include <menps/medev2/ucx/uct/tl_resource_list.hpp>
#include <menps/medev2/ucx/uct/memory_domain.hpp>
#include <menps/medev2/ucx/uct/memory.hpp>
#include <menps/medev2/ucx/uct/md_config.hpp>
#include <menps/medev2/ucx/uct/iface_config.hpp>
#include <menps/medev2/ucx/uct/interface.hpp>
#include <menps/medev2/ucx/uct/endpoint.hpp>
#include <menps/medev2/ucx/uct/address_buffer.hpp>
#include <menps/medev2/ucx/uct/remote_key.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct uct_policy
{
    using uct_facade_type = typename P::uct_facade_type;
    
    // TODO: Precisely, this is in UCS.
    using async_context_type = ucs::async_context;
    
    using worker_type = worker<P>;
    using md_resource_list_type = md_resource_list<P>;
    using tl_resource_list_type = tl_resource_list<P>;
    using memory_domain_type = memory_domain<P>;
    using memory_type = memory<P>;
    using md_config_type = md_config<P>;
    using iface_config_type = iface_config<P>;
    using interface_type = interface<P>;
    using iface_address_type = address_buffer<uct_iface_addr_t>;
    using device_address_type = address_buffer<uct_device_addr_t>;
    using ep_address_type = address_buffer<uct_ep_addr_t>;
    using endpoint_type = endpoint<P>;
    using rkey_buffer_type = address_buffer<void>;
    using remote_key_type = remote_key<P>;
    
    static memory_domain_type open_md(
        uct_facade_type&    uf
    ,   const char* const   tl_name
    ,   const char* const   dev_name
    ) {
        auto md_ress = md_resource_list_type::query(uf);
        
        for (auto&& md_res : md_ress) {
            auto md_conf = md_config_type::read(uf, md_res.md_name);
            auto md = memory_domain_type::open(uf, md_res.md_name, md_conf.get());
            
            auto tl_ress = tl_resource_list_type::query(uf, md.get());
            
            for (auto&& tl_res : tl_ress) {
                if ((strcmp(tl_res.tl_name, tl_name) == 0)
                    && (strcmp(tl_res.dev_name, dev_name) == 0))
                {
                    return md;
                }
            }
        }
        
        return {};
    }
    
    static iface_address_type get_iface_address(
        interface_type&         iface
    ,   const uct_iface_attr_t& iface_attr
    ) {
        auto addr = iface_address_type::make(iface_attr.iface_addr_len);
        iface.get_iface_address(addr.get());
        return addr;
    }
    static device_address_type get_device_address(
        interface_type&         iface
    ,   const uct_iface_attr_t& iface_attr
    ) {
        auto addr = device_address_type::make(iface_attr.device_addr_len);
        iface.get_device_address(addr.get());
        return addr;
    }
    static ep_address_type get_ep_address(
        endpoint_type&          ep
    ,   const uct_iface_attr_t& iface_attr
    ) {
        auto addr = ep_address_type::make(iface_attr.ep_addr_len);
        ep.get_ep_address(addr.get());
        return addr;
    }
    static rkey_buffer_type pack_rkey(
        memory_type&            mem
    ,   const uct_md_attr_t&    md_attr
    ) {
        auto rkey_buf = rkey_buffer_type::make(md_attr.rkey_packed_size);
        mem.pack_rkey(rkey_buf.get());
        return rkey_buf;
    }
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

