
#pragma once

#include <menps/medev2/ucx/uct/direct_facade.hpp>
#include <menps/medev2/ucx/ucs/async_context.hpp>
#include <menps/medev2/ucx/uct/worker.hpp>
#include <menps/medev2/ucx/uct/component_list.hpp>
#include <menps/medev2/ucx/uct/tl_resource_list.hpp>
#include <menps/medev2/ucx/uct/memory_domain.hpp>
#include <menps/medev2/ucx/uct/memory.hpp>
#include <menps/medev2/ucx/uct/md_config.hpp>
#include <menps/medev2/ucx/uct/iface_config.hpp>
#include <menps/medev2/ucx/uct/interface.hpp>
#include <menps/medev2/ucx/uct/endpoint.hpp>
#include <menps/medev2/ucx/uct/address_buffer.hpp>
#include <menps/medev2/ucx/uct/remote_key.hpp>
#include <menps/medev2/ucx/uct/uct_itf_id.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct uct_policy
{
    using uct_facade_type = typename P::uct_facade_type;
    using prof_aspect_type = typename P::prof_aspect_type;
    
    // TODO: Precisely, this is in UCS.
    using async_context_type = ucs::async_context;
    
    using worker_type = worker<P>;
    using component_list_type = component_list<P>;
    using md_resources_type = std::vector<uct_md_resource_desc_t>;
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
    
    static md_resources_type query_md_resources(
        component_list_type&    cl
    ,   uct_component* const    component
    ) {
        const auto num_md_ress = cl.query_md_resource_count(component);
        md_resources_type md_ress(num_md_ress);
        cl.query_md_resources(component, md_ress.data());
        return md_ress;
    }

    struct open_md_result {
        uct_component*      component;
        memory_domain_type  md;
    };

    static open_md_result open_md(
        uct_facade_type&    uf
    ,   const char* const   tl_name
    ,   const char* const   dev_name
    ) {
        auto components = component_list_type::query(uf);
        
        for (auto&& component : components) {
            auto md_ress = query_md_resources(components, component);
            
            // TODO: uct_md_config_read() doesn't require uct_md_h.
            auto md_conf = md_config_type::read(
                uf, component, nullptr, nullptr);
            
            for (auto&& md_res : md_ress) {
                auto md = memory_domain_type::open(
                    uf, component, md_res.md_name, md_conf.get());
                
                auto tl_ress = tl_resource_list_type::query(uf, md.get());
                
                for (auto&& tl_res : tl_ress) {
                    if ((strcmp(tl_res.tl_name, tl_name) == 0)
                        && (strcmp(tl_res.dev_name, dev_name) == 0))
                    {
                        return { component, mefdn::move(md) };
                    }
                }
            }
        }
        return { nullptr, {} };
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

template <typename UltItf>
struct direct_uct_itf_policy
{
private:
    using facade_policy_type = direct_uct_facade_policy<UltItf>;

public:
    using uct_facade_type = direct_uct_facade<facade_policy_type>;
    using prof_aspect_type = typename facade_policy_type::prof_aspect_type;
};

template <typename UltItf>
struct direct_uct_itf
    : uct_policy<direct_uct_itf_policy<UltItf>>
{
    using ult_itf_type = UltItf;
};


template <typename P>
struct get_uct_itf_type<uct_itf_id_t::DIRECT, P>
    : mefdn::type_identity<
        direct_uct_itf<typename P::ult_itf_type>
    >
{ };

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

