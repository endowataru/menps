
#pragma once

#include "worker_command_queue.hpp"
#include <menps/meult/offload/basic_cv_offload_thread.hpp>

namespace menps {
namespace meucp {

// - blocking or non-blocking
// - worker or endpoint or others

class worker_thread;

struct worker_thread_policy
    : meucp::ult_policy
{
    typedef worker_thread   derived_type;
};

//template <typename Policy>
class worker_thread
    : public meult::basic_cv_offload_thread<worker_thread_policy>
{
    //MEFDN_POLICY_BASED_CRTP(Policy)
    
public:
    explicit worker_thread(worker_command_queue& que, ucp_worker_h wk)
        : que_(que)
        , wk_(wk)
    {
        this->start();
    }
    
    ~worker_thread()
    {
        this->stop();
    }
    
private:
    friend class meult::basic_offload_thread<worker_thread_policy>;
    friend class meult::basic_cv_offload_thread<worker_thread_policy>;
    
    void force_notify()
    {
        que_.force_notify();
    }
    
    worker_command_queue::dequeue_transaction try_dequeue()
    {
        return que_.try_dequeue(1);
    }
    
    bool try_sleep()
    {
        return que_.try_sleep();
    }
    
    bool try_execute(worker_command& c)
    {
        switch (c.code)
        {
            
            /*#define dEF_BASE(name) \
                case worker_command_code::name: { \
                    auto& p = reinterpret_cast<name ## _command&>(c.cmd); \
                    execute_ ## name(wk_, p); \
                    break; \
                }*/
            
            #define X0(name, tr)                                                            DEF_BASE(name)
            #define X1(name, tr, t0, a0)                                                    DEF_BASE(name)
            #define X2(name, tr, t0, a0, t1, a1)                                            DEF_BASE(name)
            #define X3(name, tr, t0, a0, t1, a1, t2, a2)                                    DEF_BASE(name)
            #define X4(name, tr, t0, a0, t1, a1, t2, a2, t3, a3)                            DEF_BASE(name)
            #define X5(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4)                    DEF_BASE(name)
            #define X6(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)            DEF_BASE(name)
            #define X7(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6)    DEF_BASE(name)
            
            #define DEF_BASE(name) \
                case worker_command_code::name: { \
                    auto& cmd = reinterpret_cast<name ## _command&>(c.cmd); \
                    const auto ret = call_ ## name(wk_, cmd.params); \
                    cmd.nt.ch->set_value(ret); \
                    break; \
                }
            
            MEUCP_WORKER_FUNCS_SYNC_STATUS()
            MEUCP_WORKER_FUNCS_SYNC_STATUS_PTR()
            MEUCP_WORKER_FUNCS_SYNC_TAG_MESSAGE_PTR()
            MEUCP_ENDPOINT_FUNCS_SYNC_STATUS()
            MEUCP_ENDPOINT_FUNCS_SYNC_PTR()
            MEUCP_REQUEST_FUNCS_SYNC_STATUS()
            
            #undef DEF_BASE
            
            #define DEF_BASE(name) \
                case worker_command_code::name: { \
                    auto& cmd = reinterpret_cast<name ## _command&>(c.cmd); \
                    call_ ## name(wk_, cmd.params); \
                    cmd.nt.ch->set_value(); \
                    break; \
                }
            
            MEUCP_WORKER_FUNCS_SYNC_VOID()
            MEUCP_ENDPOINT_FUNCS_SYNC_VOID()
            
            #undef DEF_BASE
            
            #define DEF_BASE(name) \
                case worker_command_code::name: { \
                    auto& cmd = reinterpret_cast<name ## _command&>(c.cmd); \
                    const auto ret = call_ ## name(wk_, cmd.params); \
                    check_error(ret); \
                    break; \
                }
            
            MEUCP_ENDPOINT_FUNCS_ASYNC_STATUS()
            
            #undef DEF_BASE
            
            #undef X0
            #undef X1
            #undef X2
            #undef X3
            #undef X4
            #undef X5
            #undef X6
            #undef X7
        }
        
        return true; // TODO
    }
    
    void check_error(ucs_status_t s) {
        // ignore
    }
    
    bool has_remaining()
    {
        return false; // TODO
        //return ! proc_indexes_.empty();
    }
    
    void post_all()
    {
        // TODO
        
        /*const auto proc_first = conf_.proc_first;
        
        for (auto itr = proc_indexes_.begin(); itr != proc_indexes_.end(); )
        {
            const auto proc_index = *itr;
            
            const auto proc = proc_first + proc_index;
            MEFDN_ASSERT(proc < proc_first + conf_.num_procs);
            
            if (qps_[proc_index]->try_post_all()) {
                // Erase the corresponding process ID from process index list.
                itr = proc_indexes_.erase(itr);
            }
            else {
                // Increment the iterator
                // only when there are at least one WR for that process (ID).
                ++itr;
            }
        }*/
    }
    
    worker_command_queue& que_;
    ucp_worker_h wk_;
};

} // namespace meucp
} // namespace menps

