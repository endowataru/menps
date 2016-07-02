
#pragma once

#include "verbs.hpp"
#include "params.hpp"

namespace mgcom {
namespace ibv {

struct send_work_request
    : ibv_send_wr
{
    void set_write(const write_params& params, ibv_sge& sge)
    {
        this->wr_id                 = params.wr_id;
        //this->next                = MGBASE_NULLPTR;
        this->sg_list               = &sge;
        this->num_sge               = 1;
        this->opcode                = IBV_WR_RDMA_WRITE;
        this->send_flags            = 0; // TODO
        this->wr.rdma.remote_addr   = params.raddr;
        this->wr.rdma.rkey          = params.rkey;
    }
    
    void set_read(const read_params& params, ibv_sge& sge)
    {
        this->wr_id                 = params.wr_id;
        //this->next                = MGBASE_NULLPTR;
        this->sg_list               = &sge;
        this->num_sge               = 1;
        this->opcode                = IBV_WR_RDMA_READ;
        this->send_flags            = 0; // TODO
        this->wr.rdma.remote_addr   = params.raddr;
        this->wr.rdma.rkey          = params.rkey;
    }
    
    void set_compare_and_swap(const compare_and_swap_params& params, ibv_sge& sge)
    {
        this->wr_id                 = params.wr_id;
        //this->next                  = MGBASE_NULLPTR;
        this->sg_list               = &sge;
        this->num_sge               = 1;
        this->opcode                = IBV_WR_ATOMIC_CMP_AND_SWP;
        this->send_flags            = 0; // TODO
        this->wr.atomic.remote_addr = params.raddr;
        this->wr.atomic.compare_add = params.expected;
        this->wr.atomic.swap        = params.desired;
        this->wr.atomic.rkey        = params.rkey;
    }
    
    void set_fetch_and_add(const fetch_and_add_params& params, ibv_sge& sge)
    {
        this->wr_id                 = params.wr_id;
        //this->next                  = MGBASE_NULLPTR;
        this->sg_list               = &sge;
        this->num_sge               = 1;
        this->opcode                = IBV_WR_ATOMIC_FETCH_AND_ADD;
        this->send_flags            = 0; // TODO
        this->wr.atomic.remote_addr = params.raddr;
        this->wr.atomic.compare_add = params.value;
        // this->wr.atomic.swap is unused
        this->wr.atomic.rkey        = params.rkey;
    }
};

} // namespace ibv
} // namespace mgcom

