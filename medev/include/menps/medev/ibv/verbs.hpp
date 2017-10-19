
#pragma once

#include <menps/medev/common.hpp>

#include "infiniband/verbs.h"

#ifdef MEDEV_IBV_EXP_SUPPORTED
    #include "infiniband/verbs_exp.h"
#endif

namespace menps {
namespace medev {
namespace ibv {

typedef mefdn::uint16_t    lid_t;

struct node_id_t
{
    lid_t lid;
};

typedef mefdn::uint8_t     port_num_t;
typedef mefdn::uint32_t    qp_num_t;

struct global_qp_id
{
    node_id_t   node_id;
    port_num_t  port_num;
    qp_num_t    qp_num;
};

typedef mefdn::uint64_t    wr_id_t;

inline node_id_t make_node_id_from_lid(const mefdn::uint16_t lid) {
    return { lid };
}

} // namespace ibv
} // namespace medev
} // namespace menps

