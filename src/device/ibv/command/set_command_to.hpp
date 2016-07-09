
#pragma once

#include "make_wr_to.hpp"
#include "command.hpp"

namespace mgcom {
namespace ibv {

inline void set_command_to(
    const command&      cmd
,   ibv_send_wr* const  wr
,   ibv_sge* const      sge
)
{
    switch (cmd.code)
    {
        case command_code::ibv_read:
            make_wr_to(*reinterpret_cast<const read_params*>(cmd.arg), wr, sge);
            break;
        
        case command_code::ibv_write:
            make_wr_to(*reinterpret_cast<const write_params*>(cmd.arg), wr, sge);
            break;
        
        case command_code::ibv_compare_and_swap:
            make_wr_to(*reinterpret_cast<const compare_and_swap_params*>(cmd.arg), wr, sge);
            break;
        
        case command_code::ibv_fetch_and_add:
            make_wr_to(*reinterpret_cast<const fetch_and_add_params*>(cmd.arg), wr, sge);
            break;
        
        MGBASE_COVERED_SWITCH()
    }
}

} // namespace ibv
} // namespace mgcom

