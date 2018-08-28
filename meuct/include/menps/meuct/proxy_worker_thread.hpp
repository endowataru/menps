
#pragma once

#include <menps/meuct/proxy_funcs.hpp>
#include <menps/medev2/ucx/ucx_error.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meuct {

template <typename P>
class proxy_worker_thread
    : public P::offload_thread_base_type
{
    using proxy_worker_type = typename P::proxy_worker_type;
    using proxy_iface_type = typename P::proxy_iface_type;
    using proxy_endpoint_type = typename P::proxy_endpoint_type;
    
    using proxy_completion_pool_type = typename P::proxy_completion_pool_type;
    using proxy_completion_type = typename P::proxy_completion_type;
    
    using command_type = typename P::command_type;
    using command_code_type = typename P::command_code_type;
    using command_queue_type = typename P::command_queue_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    
public:
    explicit proxy_worker_thread(proxy_worker_type& pr_wk, command_queue_type& que)
        : pr_wk_(pr_wk)
        , que_(que)
    {
        this->start();
    }
    ~proxy_worker_thread()
    {
        //this->stop();
        
        this->stop_safe();
    }
    
private:
    // TODO: workaround to avoid the bug of basic_uncond_offload_queue
    void stop_safe()
    {
        while (true) {
            auto t = this->que_.try_enqueue(1, true);
            
            if (MEFDN_LIKELY(t.valid()))
            {
                auto& dest = *t.begin();
                
                dest.code = command_code_type::finalize;
                dest.notif = nullptr;
                
                t.commit(1);
                
                this->que_.notify_if_sleeping(t);
                
                break;
            }
            
            ult_itf_type::this_thread::yield();
        }
        
        this->th_.join();
    }
    
    // TODO: Too crude.
    friend typename P::offload_thread_base_base_type;
    friend typename P::offload_thread_base_type;
    
    void force_notify()
    {
        this->que_.force_notify();
    }
    
    typename command_queue_type::dequeue_transaction try_dequeue()
    {
        return this->que_.try_dequeue(1);
    }
    
    bool try_sleep()
    {
        return this->que_.try_sleep();
    }
    
    bool try_execute(command_type& cmd)
    {
        auto uf = this->pr_wk_.get_orig_facade();
        
        bool is_success = true;
        
        switch (cmd.code) {
            #define REPLACE_IFACE \
                auto& pr_iface = *reinterpret_cast<proxy_iface_type*>(params.iface); \
                auto& orig_iface = pr_iface.get_orig_iface(); \
                \
                /* Overwrite the iface pointer. */ \
                params.iface = orig_iface.get();
            
            #define REPLACE_EP \
                auto& pr_ep = *reinterpret_cast<proxy_endpoint_type*>(params.ep); \
                auto& orig_ep = pr_ep.get_orig_endpoint(); \
                \
                /* Overwrite the endpoint pointer. */ \
                params.ep = orig_ep.get();
            
            #define REPLACE_COMP(pr_cb, pr_ptr) \
                const auto pc = \
                    this->create_proxy_completion( \
                        pr_cb, pr_ptr, params.comp); \
                \
                /* Overwrite the completion handle. */ \
                params.comp = &pc->pr_comp;
            
            #define D(dummy, name, tr, num, ...) \
                case command_code_type::name: { \
                    auto params = \
                        reinterpret_cast<medev2::ucx::uct::name ## _params&>(cmd.params); \
                    REPLACE \
                    MEFDN_LOG_VERBOSE("msg:Executing uct_" #name " in offloading thread."); \
                    const auto ret = uf.name(params); \
                    AFTER_EXEC \
                    break; \
                }
            
            // sync
            #define AFTER_EXEC \
                this->notify_value(cmd.notif, ret);
                
                // sync (iface)
                #define REPLACE REPLACE_IFACE
                MEDEV2_UCT_IFACE_FUNCS_SYNC_STATUS(D, /*dummy*/)
                #undef REPLACE
                
                // sync (ep)
                #define REPLACE REPLACE_EP
                MEDEV2_UCT_EP_FUNCS_SYNC_STATUS(D, /*dummy*/)
                #undef REPLACE
            
            #undef AFTER_EXEC
            
            #define AFTER_EXEC_ASYNC \
                is_success = this->check_async_error(ret);
            
            #define AFTER_EXEC_COMP \
                if (!is_success) this->destroy_proxy_completion(pc);
            
            #define AFTER_EXEC_POST \
                if (ret == UCS_INPROGRESS) pr_ep.increment_ongoing();
            
                // async (ep, post, implicit)
                #define REPLACE     REPLACE_EP
                #define AFTER_EXEC  AFTER_EXEC_ASYNC AFTER_EXEC_POST
                MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_IMPLICIT(D, /*dummy*/)
                #undef REPLACE
                #undef AFTER_EXEC
                
                // async (ep, post completion)
                #define REPLACE REPLACE_EP \
                    REPLACE_COMP(&on_complete_ep_decrement, &pr_ep)
                #define AFTER_EXEC  AFTER_EXEC_ASYNC AFTER_EXEC_POST AFTER_EXEC_COMP
                MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_COMPLETION(D, /*dummy*/)
                #undef REPLACE
            
            #undef AFTER_EXEC
            #undef AFTER_EXEC_POST
            
            // async flush
            #define AFTER_EXEC  AFTER_EXEC_ASYNC
            
                // iface_flush
                #define REPLACE REPLACE_IFACE \
                    REPLACE_COMP(&on_complete_iface_flush, &pr_iface)
                MEDEV2_UCT_IFACE_FUNCS_IFACE_FLUSH(D, /*dummy*/)
                #undef REPLACE
                
                // ep_flush
                #define REPLACE REPLACE_EP \
                    REPLACE_COMP(&on_complete_ep_flush, &pr_ep)
                MEDEV2_UCT_EP_FUNCS_EP_FLUSH(D, /*dummy*/)
                #undef REPLACE
            
            #undef AFTER_EXEC
            
            #undef AFTER_EXEC_ASYNC
            #undef AFTER_EXEC_COMP
            
            #undef REPLACE_IFACE
            #undef REPLACE_EP
            #undef REPLACE_COMP
            #undef D
            
            case command_code_type::finalize: {
                this->finished_.store(true);
                break;
            }
            
            MEFDN_COVERED_SWITCH()
        }
        
        return is_success;
    }
    
    bool check_async_error(const ucs_status_t st) {
        if (st == UCS_OK || st == UCS_INPROGRESS) {
            return true;
        }
        else if (st == UCS_ERR_NO_RESOURCE) {
            return false;
        }
        else {
            throw medev2::ucx::ucx_error("error in offloading", st);
        }
    }
    
    template <typename T>
    static void notify_value(void* const notif_void, const T value)
    {
        using async_channel_type =
            typename ult_itf_type::template async_channel<T>;
        
        const auto notif = reinterpret_cast<async_channel_type*>(notif_void);
        MEFDN_ASSERT(notif != nullptr);
        
        notif->set_value(value); \
    }
    
    static void on_complete_ep_decrement(
        uct_completion_t* const comp
    ,   const ucs_status_t      status
    ) {
        // TODO: Use container_of().
        const auto pc = reinterpret_cast<proxy_completion_type*>(comp);
        
        auto& ep = *static_cast<proxy_endpoint_type*>(pc->pr_ptr);
        ep.decrement_ongoing();
        
        on_complete_common(pc, status);
    }
    static void on_complete_ep_flush(
        uct_completion_t* const comp
    ,   const ucs_status_t      status
    ) {
        // TODO: Use container_of().
        const auto pc = reinterpret_cast<proxy_completion_type*>(comp);
        
        auto& ep = *static_cast<proxy_endpoint_type*>(pc->pr_ptr);
        ep.flush_ongoing();
        
        on_complete_common(pc, status);
    }
    static void on_complete_iface_flush(
        uct_completion_t* const comp
    ,   const ucs_status_t      status
    ) {
        // TODO: Use container_of().
        const auto pc = reinterpret_cast<proxy_completion_type*>(comp);
        
        auto& iface = *static_cast<proxy_iface_type*>(pc->pr_ptr);
        iface.flush_ongoing();
        
        on_complete_common(pc, status);
    }
    
    static void on_complete_common(
        proxy_completion_type* const    pc
    ,   const ucs_status_t              status
    ) {
        auto& self = *pc->self;
        
        const auto orig_comp = pc->orig_comp;
        MEFDN_ASSERT(orig_comp != nullptr);
        
        MEFDN_LOG_VERBOSE(
            "msg:Complete communication in offloading thread.\t"
            "num_ongoing:{}"
        ,   self.pr_wk_.get_num_ongoing()
        );
        
        if (--orig_comp->count == 0) {
            orig_comp->func(orig_comp, status);
        }
        
        self.destroy_proxy_completion(pc);
    }
    
    bool has_remaining()
    {
        if (this->pr_wk_.get_num_ongoing() > 0) {
            return true;
        }
        else {
            return false;
        }
    }
    
    void post_all() {
        auto& orig_wk = this->pr_wk_.get_orig_worker();
        while (orig_wk.progress() != 0) {
            // Poll until there are completions
        }
    }
    
    proxy_completion_type* create_proxy_completion(
        const uct_completion_callback_t pr_cb
    ,   void* const                     pr_ptr
    ,   uct_completion_t* const         orig_comp
    ) {
        const auto pc = pc_pool_.allocate();
        pc->self = this;
        pc->pr_comp.count = 1;
        pc->pr_comp.func = pr_cb;
        pc->pr_ptr = pr_ptr;
        pc->orig_comp = orig_comp;
        return pc;
    }
    void destroy_proxy_completion(proxy_completion_type* const pc) {
        pc_pool_.deallocate(pc);
    }
    
    proxy_worker_type& pr_wk_;
    command_queue_type& que_;
    proxy_completion_pool_type pc_pool_;
};

} // namespace meuct
} // namespace menps

