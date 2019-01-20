
#pragma once

#include <menps/medev2/ucx/uct/uct_funcs.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

class direct_facade
{
public:
    #define D(dummy, name, tr, num, ...) \
        tr name(const name##_params& p) { \
            MEFDN_LOG_DEBUG( \
                "msg:Entering uct_" #name ".\t" \
                MEDEV2_EXPAND_PARAMS_TO_LOG_FMT(num, __VA_ARGS__) \
            ,   MEDEV2_EXPAND_PARAMS_TO_LOG_P_DOT_ARGS(num, __VA_ARGS__) \
            ); \
            return \
                uct_##name( \
                    MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS(num, __VA_ARGS__) \
                ); \
        }
    
    MEDEV2_UCT_FUNCS_ALL(D, /*dummy*/)
    
    #undef D
};

struct direct_facade_policy
{
    using uct_facade_type = direct_facade;
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

