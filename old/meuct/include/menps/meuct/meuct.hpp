
#pragma once

#include <menps/meuct/proxy_funcs.hpp>

#define A(i, t, a)  t a,
#define AL(i, t, a) t a

#define D(prefix, name, tr, num, ...) \
    extern "C" \
    tr prefix ## name( \
        MEDEV2_EXPAND_PARAMS(A, AL, num, __VA_ARGS__) \
    );


MEDEV2_UCT_FUNCS_ALL(D, meuct_)

#undef A
#undef AL
#undef D

namespace menps {
namespace meuct {

class proxy_facade
{
public:
    #define A(i, t, a)      p.a,
    #define AL(i, t, a)     p.a
    
    #define D(prefix, name, tr, num, ...) \
        static tr name(const medev2::ucx::uct::name##_params& p) { \
            return meuct_##name( \
                MEDEV2_EXPAND_PARAMS(A, AL, num, __VA_ARGS__) \
            ); \
        }
    
    MEDEV2_UCT_FUNCS_ALL(D, meuct_)
    
    #undef A
    #undef AL
    #undef D
};

struct proxy_facade_policy
{
    using uct_facade_type = proxy_facade;
};

} // namespace meuct
} // namespace menps

