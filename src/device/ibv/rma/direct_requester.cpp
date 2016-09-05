
#include "requester_base.hpp"
#include "requester.hpp"
#include "device/ibv/command/completer.hpp"
#include "device/ibv/command/atomic_buffer.hpp"
#include "device/ibv/command/code.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/command/make_wr_to.hpp"
#include "device/ibv/native/send_work_request.hpp"
#include "device/ibv/native/scatter_gather_entry.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {

namespace rma {
namespace /*unnamed*/ {

class ibv_direct_requester
    : public ibv_requester_base<ibv_direct_requester>
{
    static const mgbase::size_t max_num_completions = 102400;
    
public:
    typedef ibv::command_code   command_code_type;
    
    ibv_direct_requester(ibv::endpoint& ep, ibv::completer& comp, rma::allocator& alloc)
        : ep_(ep)
        , comp_(comp)
        , atomic_buf_(alloc, max_num_completions) { }
    
    ibv_direct_requester(const ibv_direct_requester&) = delete;
    ibv_direct_requester& operator = (const ibv_direct_requester&) = delete;
    
private:
    friend class ibv_requester_base<ibv_direct_requester>;
    
    #if 0
    ibv::completer& get_completer() const MGBASE_NOEXCEPT {
        return comp_;
    }
    ibv::atomic_buffer& get_atomic_buffer() MGBASE_NOEXCEPT {
        return atomic_buf_;
    }
    #endif
    
    template <typename Params, typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(
        const process_id_t      proc
    ,   const command_code_type //code
    ,   Func&&                  func
    ) {
        #if 0
        mgbase::uint64_t wr_id = 0;
        if (MGBASE_UNLIKELY(
            ! comp_.try_complete(&wr_id)
        )) {
            return false;
        }
        #endif
        
        auto t = comp_.try_start();
        
        if (!t.valid())
            return false;
        
        const auto wr_id = *t.begin();
        
        Params params = Params();
        std::forward<Func>(func)(&params);
        
        ibv::send_work_request wr = ibv::send_work_request();
        ibv::scatter_gather_entry sge = ibv::scatter_gather_entry();
        
        make_wr_to(params, wr_id, &wr, &sge, comp_, atomic_buf_);
        
        wr.next = MGBASE_NULLPTR;
        
        ibv_send_wr* bad_wr = MGBASE_NULLPTR;
        const bool success = ep_.try_post_send(proc, wr, &bad_wr);
        
        if (success) {
            t.commit();
            return true;
        }
        else {
            t.rollback();
            return false;
        }
    }
    
    mgbase::spinlock comp_lock_;
    ibv::endpoint& ep_;
    ibv::completer& comp_;
    ibv::atomic_buffer atomic_buf_;
};

} // unnamed namespace
} // namespace rma

namespace ibv {

mgbase::unique_ptr<rma::requester> make_rma_direct_requester(endpoint& ep, completer& comp, rma::allocator& alloc)
{
    return mgbase::make_unique<rma::ibv_direct_requester>(ep, comp, alloc);
}

} // namespace ibv

} // namespace mgcom

