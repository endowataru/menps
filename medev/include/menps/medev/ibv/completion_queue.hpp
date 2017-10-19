
#pragma once

#include <menps/medev/ibv/verbs.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ibv {

struct completion_queue_deleter
{
    void operator () (ibv_cq*) const noexcept;
};

class completion_queue
    : public mefdn::unique_ptr<ibv_cq, completion_queue_deleter>
{
    typedef mefdn::unique_ptr<ibv_cq, completion_queue_deleter>  base;
    
public:
    completion_queue() noexcept = default;
    
    explicit completion_queue(ibv_cq* const cq)
        : base(cq)
    { }
    
    ~completion_queue() /*noexcept*/ = default;
    
    completion_queue(const completion_queue&) = delete;
    completion_queue& operator = (const completion_queue&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(completion_queue, base)
    
    int poll(
        ibv_wc* const   wc_array
    ,   const int       num_entries
    ) {
        const int ret =
            ibv_poll_cq(this->get(), num_entries, wc_array);
        
        if (MEFDN_UNLIKELY(ret < 0)) {
            this->poll_error(ret);
        }
        
        return ret;
    }
    
private:
    void poll_error(int ret);
};

completion_queue make_completion_queue(ibv_context* ctx);

completion_queue make_completion_queue(ibv_context* ctx, int num_cqe);

} // namespace ibv
} // namespace medev
} // namespace menps

