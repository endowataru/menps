
#pragma once

#include <mgdev/common.hpp>

#include "infiniband/verbs.h"

#ifdef MGDEV_IBV_EXP_SUPPORTED
    #include "infiniband/verbs_exp.h"
#endif

namespace mgdev {
namespace ibv {

typedef mgbase::uint16_t    lid_t;

struct node_id_t
{
    lid_t lid;
};

typedef mgbase::uint8_t     port_num_t;
typedef mgbase::uint32_t    qp_num_t;

struct global_qp_id
{
    node_id_t   node_id;
    port_num_t  port_num;
    qp_num_t    qp_num;
};

typedef mgbase::uint64_t    wr_id_t;

inline node_id_t make_node_id_from_lid(const mgbase::uint16_t lid) {
    return { lid };
}

} // namespace ibv
} // namespace mgdev

