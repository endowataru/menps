
#pragma once

#include <menps/mecom2/com/uct/basic_uct_worker_set.hpp>
#include <menps/mecom2/com/basic_worker_selector.hpp>
#include <menps/medev2/ucx/uct/uct_policy.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class uct_worker_set
    : public basic_uct_worker_set<P>
    , public basic_worker_selector<P>
{
    using base_set = basic_uct_worker_set<P>;
    using base_selector = basic_worker_selector<P>;
    
    using uct_itf_type = typename P::uct_itf_type;
    using uct_facade_type = typename uct_itf_type::uct_facade_type;
    using memory_domain_type = typename uct_itf_type::memory_domain_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    
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

template <typename UctItf>
struct uct_worker_set_policy
{
    using uct_itf_type = UctItf;
    using ult_itf_type = typename UctItf::ult_itf_type;
    using worker_num_type = mefdn::size_t;
    using proc_id_type = mefdn::size_t;
    using size_type = mefdn::size_t;
};

template <typename UctItf>
using uct_worker_set_ptr = mefdn::unique_ptr<uct_worker_set<uct_worker_set_policy<UctItf>>>;

template <typename UctItf, typename Coll>
inline uct_worker_set_ptr<UctItf>
make_uct_worker_set(
    typename UctItf::uct_facade_type&       uf
,   typename UctItf::memory_domain_type&    md
,   const uct_iface_config_t* const         iface_conf
,   const uct_iface_params_t* const         iface_params
,   Coll&                                   coll
) {
    struct conf {
        typename UctItf::uct_facade_type&       uf;
        typename UctItf::memory_domain_type&    md;
        const uct_iface_config_t*               iface_conf;
        const uct_iface_params_t*               iface_params;
        Coll&                                   coll;
    };
    
    return mefdn::make_unique<uct_worker_set<uct_worker_set_policy<UctItf>>>(
        conf{ uf, md, iface_conf, iface_params, coll });
}

} // namespace mecom2
} // namespace menps

