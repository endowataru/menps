
#pragma once

#include "meucp.hpp"
#include "worker_thread.hpp"

namespace menps {
namespace meucp {

class worker
{
public:
    explicit worker(ucp_worker_h w)
        : w_(w)
        , que_()
        , t_(que_, w_)
    { }
    
    worker(const worker&) = delete;
    worker& operator = (const worker&) = delete;
    
    ucp_worker_h real() {
        return w_;
    }
    
    #define X0(name, tr)                                                                DEF_BASE(name, tr)
    #define X1(name, tr, t0, a0)                                                        DEF_BASE(name, tr)
    #define X2(name, tr, t0, a0, t1, a1)                                                DEF_BASE(name, tr)
    #define X3(name, tr, t0, a0, t1, a1, t2, a2)                                        DEF_BASE(name, tr)
    #define X4(name, tr, t0, a0, t1, a1, t2, a2, t3, a3)                                DEF_BASE(name, tr)
    #define X5(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4)                        DEF_BASE(name, tr)
    #define X6(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)                DEF_BASE(name, tr)
    #define X7(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6)        DEF_BASE(name, tr)
    
    #define DEF_BASE(name, tr) \
        tr do_ ## name (const name ## _params& p) { \
            typename ult_policy::async_channel_<tr>::type ch; \
            post(worker_command_code::name, name ## _command{ {&ch} , p }); \
            return ch.get(ult_policy::this_thread::yield); \
        }
    
    MEUCP_WORKER_FUNCS_SYNC_STATUS()
    MEUCP_WORKER_FUNCS_SYNC_STATUS_PTR()
    MEUCP_WORKER_FUNCS_SYNC_TAG_MESSAGE_PTR()
    MEUCP_WORKER_FUNCS_SYNC_VOID()
    MEUCP_ENDPOINT_FUNCS_SYNC_STATUS()
    MEUCP_ENDPOINT_FUNCS_SYNC_VOID()
    MEUCP_ENDPOINT_FUNCS_SYNC_PTR()
    MEUCP_REQUEST_FUNCS_SYNC_STATUS()
    
    #undef DEF_BASE
    
    #define DEF_BASE(name, tr) \
        void post_ ## name (const name ## _params& p) { \
            post(worker_command_code::name, name ## _command{ p }); \
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
    
    /*
        sync 
        tr post_ ## name() {
            bool flag = false;
            tr ret = tr();
            post(..., &flag);
            while (!flag) { yield(); }
            return ret;
        }
        
        async
        tr post_ ## name () {
            post();
            return UCS_INPROGRESS;
        }
    */
    
    
private:
    template <typename Cmd>
    void post(const worker_command_code code, const Cmd& cmd)
    {
        auto& que = this->que_;
        
        while (true) {
            auto t = que.try_enqueue(1, true);
            
            if (MEFDN_LIKELY(t.valid()))
            {
                auto& dest = *t.begin();
                
                dest.code = code;
                
                MEFDN_STATIC_ASSERT(sizeof(Cmd) <= worker_command::cmd_size);
                reinterpret_cast<Cmd&>(dest.cmd) = cmd;
                
                t.commit(1);
                
                que.notify_if_sleeping(t);
                
                return;
            }
            
            ult_policy::this_thread::yield();
        }

    }
    
    
    /*#define DEF_BASE(name, tr, args) \
        tr post_ ## name (const name ## _params& p) \
        { \
            if (true) { \
                return ucp_##name(FIRST_ARG, args); \
            } else { \
                \
            } \
        }
    */
    
    #if 0
    
    ult::async_status<void>
    enqueue(const worker_command_code code, const Params& params)
    {
        while (true)
        {
            auto t = que.try_enqueue(1, true);
            
            if (MEFDN_LIKELY(t.valid()))
            {
                auto& dest = *t.begin();
                
                if (t.is_sleeping()) {
                    
                    
                    return ult::make_async_ready<void>();
                }
                else {
                    dest.code = code;
                    
                    MEFDN_STATIC_ASSERT(sizeof(Params) <= command::args_size);
                    reinterpret_cast<Params&>(dest.arg) = params;
                    
                    t.commit(1);
                    
                    que.notify_if_sleeping(t);
                    
                    return ult::make_async_deferred<void>();
                }
            }
            
            ult::this_thread::yield();
        }
    }
    #endif
    
private:
    ucp_worker_h w_;
    worker_command_queue que_;
    worker_thread t_;
};

} // namespace meucp
} // namespace menps

