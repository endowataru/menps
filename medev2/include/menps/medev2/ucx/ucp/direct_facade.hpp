
#pragma once

#include <menps/medev2/ucx/ucp/ucp_params.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

class direct_facade
{
public:
    #define A1(t0, a0)      p.a0
    #define A2(t0, a0, ...) p.a0, A1(__VA_ARGS__)
    #define A3(t0, a0, ...) p.a0, A2(__VA_ARGS__)
    #define A4(t0, a0, ...) p.a0, A3(__VA_ARGS__)
    #define A5(t0, a0, ...) p.a0, A4(__VA_ARGS__)
    #define A6(t0, a0, ...) p.a0, A5(__VA_ARGS__)
    #define A7(t0, a0, ...) p.a0, A6(__VA_ARGS__)
    #define A8(t0, a0, ...) p.a0, A7(__VA_ARGS__)
    
    #define D(name, tr, num, ...) \
        static tr name(const name##_params& p) { \
            return ucp_##name(A##num(__VA_ARGS__)); \
        }
    
    MEDEV2_UCP_FUNCS_ALL(D)
    
    #undef D
    #undef A1
    #undef A2
    #undef A3
    #undef A4
    #undef A5
    #undef A6
    #undef A7
    #undef A8
};

struct direct_facade_policy
{
    using ucp_facade_type = direct_facade;
};

} // namespace ucp
} // namespace ucx
} // namespace medev2
} // namespace menps

