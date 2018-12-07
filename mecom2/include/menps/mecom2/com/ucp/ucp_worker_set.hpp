
#pragma once

#include <menps/mecom2/com/ucp/basic_ucp_worker_set.hpp>
#include <menps/mecom2/com/basic_worker_selector.hpp>
#include <menps/medev2/ucx/ucp/ucp_policy.hpp>
#include <menps/medev2/ucx/ucp/direct_facade.hpp>

namespace menps {
namespace mecom2 {

using default_ucp_policy =
    medev2::ucx::ucp::ucp_policy<medev2::ucx::ucp::direct_facade_policy>;

struct ucp_worker_set_policy
{
    using ucp_itf_type = default_ucp_policy;
    using ult_itf_type = default_ult_itf;
    using worker_num_type = mefdn::size_t;
    using proc_id_type = mefdn::size_t;
    using size_type = mefdn::size_t;
};

class ucp_worker_set
    : public basic_ucp_worker_set<ucp_worker_set_policy>
    , public basic_worker_selector<ucp_worker_set_policy>
{
    using policy_type = ucp_worker_set_policy;
    using base_set = basic_ucp_worker_set<policy_type>;
    using base_selector = basic_worker_selector<policy_type>;
    
    using ult_itf_type = typename policy_type::ult_itf_type;
    using ucp_itf_type = typename policy_type::ucp_itf_type;
    
    template <typename Coll>
    struct set_conf {
        ucp_itf_type::ucp_facade_type&  uf;
        ucp_itf_type::context_type&     ctx;
        const ucp_worker_params_t&      wk_params;
        Coll&                           coll;
        mefdn::size_t                   num_wks;
    };
    
    struct selector_conf {
        mefdn::size_t max_wk_num;
    };
    
public:
    template <typename Conf>
    explicit ucp_worker_set(const Conf& conf)
        : base_set(set_conf<decltype(conf.coll)>{
            conf.uf, conf.ctx, conf.wk_params, conf.coll, calc_num_ucp_wks()
        })
        , base_selector(selector_conf{ calc_num_ucp_wks() })
    { }
    
private:
    static mefdn::size_t calc_num_ucp_wks()
    {
        const auto num_ult_wks = ult_itf_type::get_num_workers();
        return std::max(num_ult_wks, 1ul);
    }
};

template <typename Coll>
inline mefdn::unique_ptr<ucp_worker_set> make_ucp_worker_set(
    ucp_worker_set_policy::ucp_itf_type::ucp_facade_type&   uf
,   ucp_worker_set_policy::ucp_itf_type::context_type&      ctx
,   const ucp_worker_params_t&                              wk_params
,   Coll&                                                   coll
) {
    struct conf {
        ucp_worker_set_policy::ucp_itf_type::ucp_facade_type&   uf;
        ucp_worker_set_policy::ucp_itf_type::context_type&      ctx;
        const ucp_worker_params_t&                              wk_params;
        Coll&                                                   coll;
    };
    
    return mefdn::make_unique<ucp_worker_set>(conf{ uf, ctx, wk_params, coll });
}

} // namespace mecom2
} // namespace menps

