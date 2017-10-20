
#pragma once

#include <menps/mecom/common.hpp>

namespace menps {
namespace mecom {
namespace ibv {

struct read_command
{
    mefdn::uint64_t            raddr;
    mefdn::uint32_t            rkey;
    mefdn::uint64_t            laddr;
    mefdn::uint32_t            lkey;
    mefdn::size_t              size_in_bytes;
    mefdn::callback<void ()>   on_complete;
};

struct write_command
{
    mefdn::uint64_t            raddr;
    mefdn::uint32_t            rkey;
    mefdn::uint64_t            laddr;
    mefdn::uint32_t            lkey;
    mefdn::size_t              size_in_bytes;
    mefdn::callback<void ()>   on_complete;
};

struct atomic_read_command
{
    mefdn::uint64_t            raddr;
    mefdn::uint32_t            rkey;
    mefdn::uint64_t*           dest_ptr;
    mefdn::callback<void ()>   on_complete;
};

struct atomic_write_command
{
    mefdn::uint64_t            raddr;
    mefdn::uint32_t            rkey;
    mefdn::uint64_t            value;
    mefdn::callback<void ()>   on_complete;
};

struct compare_and_swap_command
{
    mefdn::uint64_t            raddr;
    mefdn::uint32_t            rkey;
    mefdn::uint64_t            expected;
    mefdn::uint64_t            desired;
    mefdn::uint64_t*           result_ptr;
    mefdn::callback<void ()>   on_complete;
};

struct fetch_and_add_command
{
    mefdn::uint64_t            raddr;
    mefdn::uint32_t            rkey;
    mefdn::uint64_t            value;
    mefdn::uint64_t*           result_ptr;
    mefdn::callback<void ()>   on_complete;
};

} // namespace ibv
} // namespace mecom
} // namespace menps

