
#pragma once

#include <mgcom/rma.hpp>
#include <mgcom/rpc.hpp>
#include <common/rma/rma.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace mpi {

template <typename Policy>
class emulated_atomic
    : public virtual Policy::requester_interface_type
{
    typedef typename Policy::derived_type       derived_type;
    typedef typename Policy::atomic_value_type  atomic_value_type;
    typedef typename Policy::handler_id_type    handler_id_type;

public:
    emulated_atomic()
    {
    }
    
protected:
    void setup()
    {
        auto& self = this->derived();
        auto& rpc_rqstr = self.get_rpc_requester();
        
        Policy::register_handler(rpc_rqstr, atomic_read_handler{ });
        Policy::register_handler(rpc_rqstr, atomic_write_handler{ });
        Policy::register_handler(rpc_rqstr, fetch_and_add_handler{ });
        Policy::register_handler(rpc_rqstr, compare_and_swap_handler{ });
    }

private:
    class atomic_read_handler
    {
    public:
        static const handler_id_type handler_id = 2000; // TODO: remove magic numbers
        
        struct request_type {
            const atomic_value_type* ptr;
        };
        typedef atomic_value_type    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            const auto& rqst = sc.request();
            
            // TODO : memory ordering
            const auto result = *rqst.ptr;
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote read.\t"
                "src_ptr:{:x}\tresult:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(rqst.ptr)
            ,   result
            );
            
            auto rply = sc.make_reply();
            *rply = result;
            
            return rply;
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_read(
        const typename Policy::async_atomic_read_params_type&   params
    ) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        
        const typename atomic_read_handler::request_type rqst{
            to_raw_pointer(params.src_rptr)
        };
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote read.\t"
            "src_proc:{}\tsrc_rptr:{:x}"
        ,   params.src_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(rqst.ptr)
        );
        
        // TODO: make this async
        const auto rply_msg =
            Policy::template call<atomic_read_handler>(
                self.get_rpc_requester()
            ,   params.src_proc
            ,   rqst
            );
        
        *params.dest_ptr = *rply_msg;
        
        return ult::make_async_ready(); // TODO
    }
    
private:
    class atomic_write_handler
    {
    public:
        static const handler_id_type handler_id = 2002;
        
        struct request_type {
            atomic_value_type*   ptr;
            atomic_value_type    value;
        };
        typedef void    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            const auto& req = sc.request();
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote write.\t"
                "value:{}\tdest_ptr:{:x}"
            ,   req.value
            ,   reinterpret_cast<mgbase::uintptr_t>(req.ptr)
            );
            
            *req.ptr = req.value;
            
            return sc.make_reply();
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_write(
        const typename Policy::async_atomic_write_params_type& params
    ) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        
        const typename atomic_write_handler::request_type rqst{
            to_raw_pointer(params.dest_rptr)
        ,   params.value
        };
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote write.\t"
            "dest_proc:{}\tdest_rptr:{:x}\tvalue:{}"
        ,   params.dest_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(rqst.ptr)
        ,   rqst.value
        );
        
        // TODO: make this async
        Policy::template call<atomic_write_handler>(
            self.get_rpc_requester()
        ,   params.dest_proc
        ,   rqst
        );
        
        return ult::make_async_ready(); // TODO
    }
    
private:
    class compare_and_swap_handler
    {
    public:
        static const handler_id_type handler_id = 2004;
        
        struct request_type {
            atomic_value_type*  target;
            atomic_value_type   expected;
            atomic_value_type   desired;
        };
        typedef atomic_value_type    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            const auto& rqst = sc.request();
            
            // TODO
            const auto target =
                reinterpret_cast<mgbase::atomic<atomic_value_type>*>(rqst.target);
            
            auto expected = rqst.expected;
            
            target->compare_exchange_strong(expected, rqst.desired);
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote compare and swap.\t"
                "target_addr:{:x}\texpected:{}\tdesired:{}\tresult:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(rqst.target)
            ,   rqst.expected
            ,   rqst.desired
            ,   expected
            );
            
            auto rply = sc.make_reply();
            
            // Return the old value.
            *rply = expected;
            
            return rply;
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_compare_and_swap(
        const typename Policy::async_compare_and_swap_params_type&  params
    ) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        
        const typename compare_and_swap_handler::request_type rqst{
            to_raw_pointer(params.target_rptr)
        ,   params.expected
        ,   params.desired
        };
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote compare and swap.\t"
            "proc:{:x}\ttarget:{:x}\texpected:{}\tdesired:{}\tresult_ptr:{:x}"
        ,   params.target_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(rqst.target)
        ,   rqst.expected
        ,   rqst.desired
        ,   reinterpret_cast<mgbase::uintptr_t>(params.result_ptr)
        );
        
        // TODO: make this async
        auto rply_msg =
            Policy::template call<compare_and_swap_handler>(
                self.get_rpc_requester()
            ,   params.target_proc
            ,   rqst
            );
        
        *params.result_ptr = *rply_msg;
        
        return ult::make_async_ready();
    }
    
private:
    class fetch_and_add_handler
    {
    public:
        static const handler_id_type handler_id = 2006;
        
        struct request_type {
            atomic_value_type*   target;
            atomic_value_type    diff;
        };
        typedef atomic_value_type    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            const auto& rqst = sc.request();
            
            const auto target =
                reinterpret_cast<mgbase::atomic<atomic_value_type>*>(rqst.target);
            
            // Returns the old value.
            const auto result = target->fetch_add(rqst.diff);
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote fetch and add.\t"
                "target_addr:{:x}\tdiff:{}\tresult:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(rqst.target)
            ,   rqst.diff
            ,   result
            );
            
            auto rply = sc.make_reply();
            
            // Return the old value.
            *rply = result;
            
            return rply;
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_fetch_and_add(
        const typename Policy::async_fetch_and_add_params_type& params
    ) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        
        const typename fetch_and_add_handler::request_type rqst{
            to_raw_pointer(params.target_rptr)
        ,   params.value
        };
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote fetch and add.\t"
            "proc:{:x}\ttarget_addr:{:x}\tdiff:{}\tresult_ptr:{:x}"
        ,   params.target_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(rqst.target)
        ,   rqst.diff
        ,   reinterpret_cast<mgbase::uintptr_t>(params.result_ptr)
        );
        
        // TODO: make this async
        auto rply_msg =
            Policy::template call<fetch_and_add_handler>(
                self.get_rpc_requester()
            ,   params.target_proc
            ,   rqst
            );
        
        *params.result_ptr = *rply_msg;
        
        return ult::make_async_ready();
    }
    
private:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
};

} // namespace mpi
} // namespace mgcom

