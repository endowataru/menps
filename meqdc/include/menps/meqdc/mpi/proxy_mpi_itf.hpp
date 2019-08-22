
#pragma once

#include <menps/meqdc/common.hpp>
#include <menps/meqdc/mpi/proxy_mpi_facade.hpp>
#include <menps/meqdc/mpi/proxy_mpi_request_holder.hpp>
#include <menps/medev2/mpi/mpi_funcs.hpp>
#include <menps/medev2/mpi/mpi_itf_id.hpp>
#include <menps/medev2/mpi/direct_mpi_facade.hpp>

namespace menps {
namespace meqdc {

union proxy_mpi_params {
    #define D(dummy, name, Name, tr, num, ...) \
        medev2::mpi::name##_params name; \
    
    // request-based non-blocking calls
    MEDEV2_MPI_P2P_NONBLOCK_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_REQ_BASED_RMA_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_COLLECTIVE_NONBLOCK_FUNCS(D, /*dummy*/)
    
    // non-request-based non-blocking calls
    MEDEV2_MPI_RMA_FUNCS(D, /*dummy*/)
    
    #undef D
};

enum class proxy_mpi_code {
    inv_op = 0,
    
    #define D(dummy, name, Name, tr, num, ...) \
        name,
    
    // request-based non-blocking calls
    MEDEV2_MPI_P2P_NONBLOCK_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_REQ_BASED_RMA_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_COLLECTIVE_NONBLOCK_FUNCS(D, /*dummy*/)
    
    // non-request-based non-blocking calls
    MEDEV2_MPI_RMA_FUNCS(D, /*dummy*/)
    
    #undef D
};

enum class proxy_mpi_request_state
{
    created = 1
,   waiting
,   finished
};

template <typename P>
struct proxy_mpi_request
{
private:
    using ult_itf_type = typename P::ult_itf_type;
    using uncond_variable_type = typename ult_itf_type::uncond_variable;
    using atomic_bool_type = typename ult_itf_type::template atomic<bool>;
    using atomic_state_type = typename ult_itf_type::template atomic<proxy_mpi_request_state>;
    
public:
    proxy_mpi_request()
        : state(proxy_mpi_request_state::created)
    { }
    
    atomic_state_type       state;
    uncond_variable_type    uv;
    atomic_bool_type        finished;
    MPI_Request             orig_req;
    MPI_Status              status;
};

template <typename P>
struct proxy_mpi_request_pool_policy
{
    using element_type = proxy_mpi_request<P>;
    using ult_itf_type = typename P::ult_itf_type;
    using size_type = typename P::size_type;
    
    template <typename Pool>
    static size_type get_pool_threshold(Pool& /*pool*/) {
        return MEQDC_MPI_REQUEST_POOL_MAX_ENTRIES; // TODO
    }
};

template <typename P>
struct proxy_mpi_func
{
    proxy_mpi_code          code;
    proxy_mpi_params        params;
    proxy_mpi_request<P>*   proxy_req;
};

template <typename OrigMpiItf>
struct proxy_mpi_policy_base
{
    using orig_mpi_itf_type = OrigMpiItf;
    using orig_mpi_facade_type = typename orig_mpi_itf_type::mpi_facade_type;
    using ult_itf_type = typename orig_mpi_itf_type::ult_itf_type; // TODO
    using size_type = mefdn::size_t; // TODO
};

template <typename OrigMpiItf>
struct proxy_mpi_delegator_policy
{
    using policy_base_type = proxy_mpi_policy_base<OrigMpiItf>;
    using ult_itf_type = typename OrigMpiItf::ult_itf_type;
    using delegated_func_type = proxy_mpi_func<policy_base_type>;
    
    template <typename Pool>
    static mefdn::size_t get_pool_threshold(Pool& /*pool*/) {
        return MEQDC_MPI_DELEGATOR_POOL_MAX_ENTRIES;
    }
};

template <typename OrigMpiItf>
struct proxy_mpi_policy
    : proxy_mpi_policy_base<OrigMpiItf>
{
    using policy_base_type = proxy_mpi_policy_base<OrigMpiItf>;
    using ult_itf_type = typename policy_base_type::ult_itf_type;
    
    using proxy_request_type = proxy_mpi_request<policy_base_type>;
    using proxy_request_state_type = proxy_mpi_request_state;
    
    using request_pool_type =
        typename ult_itf_type::template numbered_pool_t<
            proxy_mpi_request_pool_policy<policy_base_type>
        >;
    
    using request_holder_type = proxy_mpi_request_holder<proxy_mpi_policy>;
    using command_code_type = proxy_mpi_code;
    using proxy_params_type = proxy_mpi_params;
    
    using delegator_type =
        typename ult_itf_type::template delegator_t<
            proxy_mpi_delegator_policy<OrigMpiItf>
        >;
    
    static constexpr mefdn::size_t max_num_requests = 1<<18; // TODO
    static constexpr mefdn::size_t max_num_ongoing = 256; // TODO
    static constexpr mefdn::size_t mpi_request_offset = 1; // TODO
};

template <typename OrigMpiItf>
struct proxy_mpi_itf
{
    using mpi_facade_type = proxy_mpi_facade<proxy_mpi_policy<OrigMpiItf>>;
    using ult_itf_type = typename proxy_mpi_policy<OrigMpiItf>::ult_itf_type;
};

} // namespace meqdc

namespace medev2 {

template <typename UltItf>
struct get_mpi_itf_type<mpi_itf_id_t::QDC, UltItf>
    : mefdn::type_identity<
        meqdc::proxy_mpi_itf<
            medev2::mpi::direct_mpi_itf<UltItf>
        >
    > { };

} // namespace medev2

} // namespace menps

