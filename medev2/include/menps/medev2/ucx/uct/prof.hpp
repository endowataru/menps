
#pragma once

#include <menps/medev2/ucx/uct/uct_funcs.hpp>
#include <cmpth/prof.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

struct uct_prof_aspect_policy
{
    #define D(x, name, ...)     x(name)
    #define UCT_PROF_KINDS(x)   MEDEV2_UCT_FUNCS_ALL(D, x)
    
    CMPTH_DEFINE_PROF_ASPECT_POLICY(UCT_PROF_KINDS)
    
    #undef UCT_PROF_KINDS
    #undef D
    
    static constexpr fdn::size_t get_default_mlog_size() noexcept {
        return 1ull << 20;
    }
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

