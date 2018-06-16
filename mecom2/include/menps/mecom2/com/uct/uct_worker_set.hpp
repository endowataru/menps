
#pragma once

#include <menps/mecom2/com/uct/basic_uct_worker_set.hpp>
#include <menps/mecom2/com/ucp/basic_ucp_worker_selector.hpp>
#include <menps/medev2/ucx/uct/uct_policy.hpp>
#include <menps/medev2/ucx/uct/direct_facade.hpp>
#ifdef MECOM2_USE_MEUCT
    #include <menps/meuct/meuct.hpp>
#endif

namespace menps {
namespace mecom2 {

#ifdef MECOM2_USE_MEUCT
using default_uct_policy =
    medev2::ucx::uct::uct_policy<meuct::proxy_facade_policy>;
#else
using default_uct_policy =
    medev2::ucx::uct::uct_policy<medev2::ucx::uct::direct_facade_policy>;
#endif

struct uct_worker_set_policy
{
    using uct_itf_type = default_uct_policy;
    using ult_itf_type = default_ult_itf;
    using worker_num_type = mefdn::size_t;
    using proc_id_type = mefdn::size_t;
    using size_type = mefdn::size_t;
};

class uct_worker_set
    : public basic_uct_worker_set<uct_worker_set_policy>
    , public basic_ucp_worker_selector<uct_worker_set_policy>
{
    using policy_type = uct_worker_set_policy;
    using base_set = basic_uct_worker_set<policy_type>;
    using base_selector = basic_ucp_worker_selector<policy_type>;
    
    using uct_itf_type = typename policy_type::uct_itf_type;
    using uct_facade_type = typename uct_itf_type::uct_facade_type;
    using memory_domain_type = typename uct_itf_type::memory_domain_type;
    
    using ult_itf_type = typename policy_type::ult_itf_type;
    
    template <typename Coll>
    struct set_conf {
        uct_facade_type&                uf;
        memory_domain_type&             md;
        const uct_iface_config_t*       iface_conf;
        const uct_iface_params_t*       iface_params;
        Coll&                           coll;
        mefdn::size_t                   num_wks;
    };
    
    struct selector_conf {
        mefdn::size_t max_wk_num;
    };
    
public:
    template <typename Conf>
    explicit uct_worker_set(const Conf& conf)
        : base_set(set_conf<decltype(conf.coll)>{
            conf.uf, conf.md, conf.iface_conf, conf.iface_params,
            conf.coll, calc_num_uct_wks()
        })
        , base_selector(selector_conf{ calc_num_uct_wks() })
    { }
    
private:
    static mefdn::size_t calc_num_uct_wks()
    {
        const auto num_ult_wks = ult_itf_type::get_num_workers();
        return std::max(num_ult_wks, 1ul);
    }
};

template <typename Coll>
inline mefdn::unique_ptr<uct_worker_set> make_uct_worker_set(
    uct_worker_set_policy::uct_itf_type::uct_facade_type&       uf
,   uct_worker_set_policy::uct_itf_type::memory_domain_type&    md
,   const uct_iface_config_t* const                             iface_conf
,   const uct_iface_params_t* const                             iface_params
,   Coll&                                                       coll
) {
    struct conf {
        uct_worker_set_policy::uct_itf_type::uct_facade_type&       uf;
        uct_worker_set_policy::uct_itf_type::memory_domain_type&    md;
        const uct_iface_config_t*                                   iface_conf;
        const uct_iface_params_t*                                   iface_params;
        Coll&                                                       coll;
    };
    
    return mefdn::make_unique<uct_worker_set>(
        conf{ uf, md, iface_conf, iface_params, coll });
}

} // namespace mecom2
} // namespace menps

