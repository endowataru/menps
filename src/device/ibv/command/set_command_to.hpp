
#pragma once

#include "make_wr_to.hpp"
#include "command.hpp"

namespace mgcom {
namespace ibv {

inline void set_command_to(
    const command&          cmd
,   const mgbase::uint64_t  wr_id
,   ibv_send_wr* const      wr
,   ibv_sge* const          sge
,   tag_queue&              comp
,   atomic_buffer&          atomic_buf
)
{
    switch (cmd.code)
    {
        // TODO: safer & cleaner code...
        
        case command_code::ibv_read:
            make_wr_to(*reinterpret_cast<const read_command*>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case command_code::ibv_write:
            make_wr_to(*reinterpret_cast<const write_command*>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case command_code::ibv_atomic_read:
            make_wr_to(*reinterpret_cast<const atomic_read_command*>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case command_code::ibv_atomic_write:
            make_wr_to(*reinterpret_cast<const atomic_write_command*>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case command_code::ibv_compare_and_swap:
            make_wr_to(*reinterpret_cast<const compare_and_swap_command*>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case command_code::ibv_fetch_and_add:
            make_wr_to(*reinterpret_cast<const fetch_and_add_command*>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        MGBASE_COVERED_SWITCH()
    }
}

} // namespace ibv
} // namespace mgcom

