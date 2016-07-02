
#pragma once

#include "verbs.hpp"
#include "params.hpp"

namespace mgcom {
namespace ibv {

struct scatter_gather_entry
    : ibv_sge
{
    void set_write(const write_params& params)
    {
        this->addr   = params.laddr;
        this->length = static_cast<mgbase::uint32_t>(params.size_in_bytes);
        this->lkey   = params.lkey;
    }
    
    void set_read(const read_params& params)
    {
        this->addr   = params.laddr;
        this->length = static_cast<mgbase::uint32_t>(params.size_in_bytes);
        this->lkey   = params.lkey;
    }
    
    void set_compare_and_swap(const compare_and_swap_params& params)
    {
        this->addr   = params.laddr;
        this->length = sizeof(mgbase::uint64_t);
        this->lkey   = params.lkey;
    }
    
    void set_fetch_and_add(const fetch_and_add_params& params)
    {
        this->addr   = params.laddr;
        this->length = sizeof(mgbase::uint64_t);
        this->lkey   = params.lkey;
    }
};

} // namespace ibv
} // namespace mgcom

