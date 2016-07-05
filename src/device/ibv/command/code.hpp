
#pragma once

#define MGCOM_IBV_COMMAND_CODES(x) \
        x(ibv_read) \
    ,   x(ibv_write) \
    ,   x(ibv_compare_and_swap) \
    ,   x(ibv_fetch_and_add)

namespace mgcom {
namespace ibv {

#define DEFINE_ENUM(x)  x

enum class command_code
{
    call = 1
,   MGCOM_IBV_COMMAND_CODES(DEFINE_ENUM)
};

#undef DEFINE_ENUM

} // namespace ibv
} // namespace mgcom

