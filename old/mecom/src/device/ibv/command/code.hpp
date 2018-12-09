
#pragma once

#include <menps/mecom/common.hpp>

#define MECOM_IBV_COMMAND_CODES(x) \
        x(ibv_read) \
    ,   x(ibv_write) \
    ,   x(ibv_atomic_read) \
    ,   x(ibv_atomic_write) \
    ,   x(ibv_compare_and_swap) \
    ,   x(ibv_fetch_and_add)

namespace menps {
namespace mecom {
namespace ibv {

#define DEFINE_ENUM(x)  x

enum class command_code
{
    MECOM_IBV_COMMAND_CODES(DEFINE_ENUM)
};

#undef DEFINE_ENUM

} // namespace ibv
} // namespace mecom
} // namespace menps

