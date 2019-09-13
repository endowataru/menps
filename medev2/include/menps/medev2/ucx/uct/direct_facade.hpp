
#pragma once

#include <menps/medev2/ucx/uct/uct_funcs.hpp>
#include <menps/medev2/ucx/uct/prof.hpp>
#include <menps/mefdn/logger.hpp>
#include <cmpth/prof/prof_tag.hpp>
#include MEFDN_PP_CAT(CMPTH_PROF_HEADER_, MEDEV2_UCT_PROF_ASPECT)

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
class direct_uct_facade
{
public:
    #define D(dummy, name, tr, num, ...) \
        tr name(const name##_params& p) { \
            MEFDN_LOG_DEBUG( \
                "msg:Entering uct_" #name ".\t" \
                MEDEV2_EXPAND_PARAMS_TO_LOG_FMT(num, __VA_ARGS__) \
            ,   MEDEV2_EXPAND_PARAMS_TO_LOG_P_DOT_ARGS(num, __VA_ARGS__) \
            ); \
            CMPTH_P_PROF_SCOPE(P, name); \
            return \
                uct_##name( \
                    MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS(num, __VA_ARGS__) \
                ); \
        }
    
    MEDEV2_UCT_FUNCS_ALL(D, /*dummy*/)
    
    #undef D
};

template <typename UltItf>
struct direct_uct_facade_policy
{
    using ult_itf_type = UltItf;
    using prof_aspect_type =
        typename ult_itf_type::template prof_aspect_t<
            cmpth::prof_tag::MEDEV2_UCT_PROF_ASPECT, uct_prof_aspect_policy>;
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

