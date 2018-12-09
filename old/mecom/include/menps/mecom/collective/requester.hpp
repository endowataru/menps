
#pragma once

#include <menps/mecom/common.hpp>

namespace menps {
namespace mecom {
namespace collective {

namespace untyped {

struct broadcast_params
{
    process_id_t    root;
    void*           ptr;
    index_t         num_bytes;
};

struct allgather_params
{
    const void*     src;
    void*           dest;
    index_t         num_bytes;
};

struct alltoall_params
{
    const void*     src;
    void*           dest;
    index_t         num_bytes;
};

} // namespace untyped


class requester
{
protected:
    requester() noexcept = default;
    
public:
    virtual ~requester() = default;
    
    requester(const requester&) = delete;
    requester& operator = (const requester&) = delete;
    
    virtual void barrier() = 0;
    
    virtual void broadcast(const untyped::broadcast_params&) = 0;
    
    virtual void allgather(const untyped::allgather_params&) = 0;
    
    virtual void alltoall(const untyped::alltoall_params&) = 0;
    
    static requester& get_instance() noexcept {
        MEFDN_ASSERT(req_ != nullptr);
        return *req_;
    }
    
    static void set_instance(requester& req) {
        req_ = &req;
    }
    
private:
    static requester* req_;

};

} // namespace collective
} // namespace mecom
} // namespace menps

