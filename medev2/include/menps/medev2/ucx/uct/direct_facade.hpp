
#pragma once

#include <menps/medev2/ucx/uct/uct_funcs.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

class direct_facade
{
public:
    #define D(dummy, name, tr, num, ...) \
        static tr name(const name##_params& p) { \
            return uct_##name( \
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

