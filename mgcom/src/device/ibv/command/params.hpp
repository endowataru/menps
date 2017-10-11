
#pragma once

#include <mgcom/common.hpp>

namespace mgcom {
namespace ibv {

struct read_command
{
    mgbase::uint64_t            raddr;
    mgbase::uint32_t            rkey;
    mgbase::uint64_t            laddr;
    mgbase::uint32_t            lkey;
    mgbase::size_t              size_in_bytes;
    mgbase::callback<void ()>   on_complete;
};

struct write_command
{
    mgbase::uint64_t            raddr;
    mgbase::uint32_t            rkey;
    mgbase::uint64_t            laddr;
    mgbase::uint32_t            lkey;
    mgbase::size_t              size_in_bytes;
    mgbase::callback<void ()>   on_complete;
};

struct atomic_read_command
{
    mgbase::uint64_t            raddr;
    mgbase::uint32_t            rkey;
    mgbase::uint64_t*           dest_ptr;
    mgbase::callback<void ()>   on_complete;
};

struct atomic_write_command
{
    mgbase::uint64_t            raddr;
    mgbase::uint32_t            rkey;
    mgbase::uint64_t            value;
    mgbase::callback<void ()>   on_complete;
};

struct compare_and_swap_command
{
    mgbase::uint64_t            raddr;
    mgbase::uint32_t            rkey;
    mgbase::uint64_t            expected;
    mgbase::uint64_t            desired;
    mgbase::uint64_t*           result_ptr;
    mgbase::callback<void ()>   on_complete;
};

struct fetch_and_add_command
{
    mgbase::uint64_t            raddr;
    mgbase::uint32_t            rkey;
    mgbase::uint64_t            value;
    mgbase::uint64_t*           result_ptr;
    mgbase::callback<void ()>   on_complete;
};

} // namespace ibv
} // namespace mgcom

