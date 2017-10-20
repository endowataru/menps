
#include "requester_base.hpp"
#include "requester.hpp"
#include "device/ibv/command/atomic_buffer.hpp"
#include "device/ibv/command/code.hpp"
#include "device/ibv/native/alltoall_queue_pairs.hpp"
#include "device/ibv/command/make_wr_to.hpp"
#include <menps/medev/ibv/verbs.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace ibv {

namespace /*unnamed*/ {

class direct_rma_requester
    : public rma::ibv_requester_base<direct_rma_requester>
{
    static const mefdn::size_t max_num_completions = 102400;
    
public:
    typedef ibv::command_code   command_code_type;
    
    explicit direct_rma_requester(const requester_config& conf)
        : conf_(conf)
        , atomic_buf_(
            atomic_buffer::config{ conf.alloc, max_num_completions, conf.reply_be }
        )
        , qp_infos_(mefdn::make_unique<qp_info []>(conf.ep.number_of_processes()))
    { }
    
    direct_rma_requester(const direct_rma_requester&) = delete;
    direct_rma_requester& operator = (const direct_rma_requester&) = delete;
    
private:
    friend class rma::ibv_requester_base<direct_rma_requester>;
    
    template <typename Params, typename Func>
    MEFDN_NODISCARD
    bool try_enqueue(
        const process_id_t      proc
    ,   const command_code_type //code
    ,   Func&&                  func
    ) {
        auto& info = qp_infos_[proc];
        auto& tag_que = conf_.qps.get_tag_queue(proc, 0);
        
        mefdn::lock_guard<mefdn::spinlock> lc(info.lock);
        
        auto t = tag_que.try_start();
        
        if (!t.valid())
            return false;
        
        const auto wr_id = *t.begin();
        
        Params params = Params();
        std::forward<Func>(func)(&params);
        
        ibv_send_wr wr = ibv_send_wr();
        ibv_sge sge = ibv_sge();
        
        make_wr_to(params, wr_id, &wr, &sge, tag_que, atomic_buf_);
        
        wr.next = nullptr;
        
        ibv_send_wr* bad_wr = nullptr;
        auto& qp = conf_.qps.get_qp(proc, 0);
        
        const bool success =
            qp.try_post_send(wr, &bad_wr);
        
        if (success) {
            t.commit();
            
            #ifdef MECOM_IBV_ENABLE_SLEEP_CQ
            auto& comp_sel = conf_.qps.get_comp_sel(proc, 0);
            comp_sel.notify(1);
            #endif
            
            return true;
        }
        else {
            t.rollback();
            return false;
        }
    }
    
    struct qp_info
    {
        mefdn::spinlock    lock;
    };
    
    const requester_config          conf_;
    ibv::atomic_buffer              atomic_buf_;
    mefdn::unique_ptr<qp_info []>  qp_infos_;
};

} // unnamed namespace

mefdn::unique_ptr<rma::requester> make_rma_direct_requester(const requester_config& conf)
{
    return mefdn::make_unique<direct_rma_requester>(conf);
}

} // namespace ibv
} // namespace mecom
} // namespace menps

