
#pragma once

#include "verbs.hpp"

namespace mgcom {
namespace ibv {

struct send_work_request
    : ibv_send_wr
{
};

} // namespace ibv
} // namespace mgcom

