
#pragma once

#include "verbs.hpp"

namespace mgcom {
namespace ibv {

struct scatter_gather_entry
    : ibv_sge
{
};

} // namespace ibv
} // namespace mgcom

