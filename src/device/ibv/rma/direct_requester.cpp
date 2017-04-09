
#include "requester_base.hpp"
#include "requester.hpp"
#include "device/ibv/command/completer.hpp"
#include "device/ibv/command/atomic_buffer.hpp"
#include "device/ibv/command/code.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/command/make_wr_to.hpp"
#include "device/ibv/command/completion_selector.hpp"
#include <mgdev/ibv/verbs.hpp>
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
    
    ibv_direct_requester(ibv::endpoint& ibv_ep, ibv::completion_selector& sel, rma::allocator& alloc, endpoint& ep)
        : ep_(ibv_ep)
        , sel_(sel)
        , atomic_buf_(ibv_ep, alloc, max_num_completions)
    {
        qps_.resize(ep.number_of_processes());
        
        for (process_id_t proc = 0; proc < ep.number_of_processes(); ++proc)  {
            qps_[proc] = new qp_info;
            
            const auto qp_num = ibv_ep.get_qp_num_of_proc(proc, 0);
            
            sel.set(qp_num, qps_[proc]->comp);
        }
    }
    
    ~ibv_direct_requester() {
        MGBASE_RANGE_BASED_FOR(auto&& qp, qps_) {
            delete qp;
        }
    }
    
    ibv_direct_requester(const ibv_direct_requester&) = delete;
    ibv_direct_requester& operator = (const ibv_direct_requester&) = delete;
    
private:
    friend class ibv_requester_base<ibv_direct_requester>;
    
    template <typename Params, typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(
        const process_id_t      proc
    ,   const command_code_type //code
    ,   Func&&                  func
    ) {
        auto& info = *qps_[proc];
        
        /*const auto qp_num = ep_.get_qp_num_of_proc(proc, 0);
        
        auto& comp = sel_.get(qp_num);*/
        
        mgbase::lock_guard<mgbase::spinlock> lc(info.lock);
        
        auto& comp = info.comp;
        
        auto t = comp.try_start();
        
        if (!t.valid())
            return false;
        
        const auto wr_id = *t.begin();
        
        Params params = Params();
        std::forward<Func>(func)(&params);
        
        ibv_send_wr wr = ibv_send_wr();
        ibv_sge sge = ibv_sge();
        
        make_wr_to(params, wr_id, &wr, &sge, comp, atomic_buf_);
        
        wr.next = MGBASE_NULLPTR;
        
        ibv_send_wr* bad_wr = MGBASE_NULLPTR;
        const bool success = ep_.try_post_send(proc, 0, wr, &bad_wr);
        
        if (success) {
            t.commit();
            return true;
        }
        else {
            t.rollback();
            return false;
        }
    }
    
    struct qp_info
    {
        mgbase::spinlock lock;
        ibv::completer comp;
    };
    
    ibv::endpoint& ep_;
    ibv::completion_selector& sel_;
    ibv::atomic_buffer atomic_buf_;
    std::vector<qp_info*> qps_;
};

} // unnamed namespace
} // namespace rma

namespace ibv {

mgbase::unique_ptr<rma::requester> make_rma_direct_requester(endpoint& ibv_ep, completion_selector& sel, rma::allocator& alloc, mgcom::endpoint& ep)
{
    return mgbase::make_unique<rma::ibv_direct_requester>(ibv_ep, sel, alloc, ep);
}

} // namespace ibv

} // namespace mgcom

