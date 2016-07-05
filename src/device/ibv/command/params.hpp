
#pragma once

#include <mgcom/common.hpp>

namespace mgcom {
namespace ibv {

struct write_params
{
    mgbase::uint64_t    wr_id;
    mgbase::uint64_t    raddr;
    mgbase::uint32_t    rkey;
    mgbase::uint64_t    laddr;
    mgbase::uint32_t    lkey;
    std::size_t         size_in_bytes;
};

struct read_params
{
    mgbase::uint64_t    wr_id;
    mgbase::uint64_t    raddr;
    mgbase::uint32_t    rkey;
    mgbase::uint64_t    laddr;
    mgbase::uint32_t    lkey;
    std::size_t         size_in_bytes;
};

struct compare_and_swap_params
{
    mgbase::uint64_t    wr_id;
    mgbase::uint64_t    raddr;
    mgbase::uint32_t    rkey;
    mgbase::uint64_t    laddr;
    mgbase::uint32_t    lkey;
    mgbase::uint64_t    expected;
    mgbase::uint64_t    desired;
};

struct fetch_and_add_params
{
    mgbase::uint64_t    wr_id;
    mgbase::uint64_t    raddr;
    mgbase::uint32_t    rkey;
    mgbase::uint64_t    laddr;
    mgbase::uint32_t    lkey;
    mgbase::uint64_t    value;
};

} // namespace ibv
} // namespace mgcom

