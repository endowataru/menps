
#pragma once

#include <mgcom/rma.hpp>
#include <mgcom/rpc.hpp>
#include <common/rma/rma.hpp>
#include "atomic.hpp"
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

template <typename T>
class emulated_atomic {
public:
    static void initialize()
    {
        mgcom::rpc::register_handler<am_read>();
        mgcom::rpc::register_handler<am_write>();
        mgcom::rpc::register_handler<am_fetch_and_add>();
        mgcom::rpc::register_handler<am_compare_and_swap>();
    }
    
    static void finalize()
    {
        // do nothing
    }

private:
    class am_read
    {
    public:
        static const mgcom::rpc::handler_id_t handler_id = 2000; // TODO: remove magic numbers
        
        struct argument_type {
            const T* ptr;
        };
        typedef T    return_type;
        
        static return_type on_request(
            const mgcom::rpc::handler_parameters& /*params*/
        ,   const argument_type& arg
        ) {
            // TODO
            const mgbase::atomic<T>* const ptr = reinterpret_cast<const mgbase::atomic<T>*>(arg.ptr);
            
            const T result = ptr->load();
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote read.\t"
                "src_ptr:{:x}\tresult:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
            ,   result
            );
            
            return result;
        }
    };
    
public:
    static bool try_read(
        const process_id_t                          src_proc
    ,   const remote_ptr<const atomic_default_t>&   src_rptr
    ,   atomic_default_t* const                     dest_ptr
    ,   const mgbase::operation&                    on_complete
    ) {
        const typename am_read::argument_type arg = { to_raw_pointer(src_rptr) };
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote read.\t"
            "src_proc:{}\tsrc_rptr:{:x}"
        ,   src_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
        );
        
        return mgcom::rpc::try_remote_call_async<am_read>(
            src_proc
        ,   arg
        ,   dest_ptr
        ,   on_complete
        );
    }
    
private:
    class am_write
    {
    public:
        static const mgcom::rpc::handler_id_t handler_id = 2002;
        
        struct argument_type {
            T*   ptr;
            T    value;
        };
        typedef void    return_type;
        
        static return_type on_request(
            const mgcom::rpc::handler_parameters& /*params*/
        ,   const argument_type& arg
        ) {
            // TODO
            mgbase::atomic<T>* const ptr = reinterpret_cast<mgbase::atomic<T>*>(arg.ptr);
            
            ptr->store(arg.value);
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote write.\t"
                "value:{}\tdest_ptr:{:x}"
            ,   arg.value
            ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
            );
            
            return;
        }
    };
    
public:
    static bool try_write(
        const process_id_t                          dest_proc
    ,   const remote_ptr<atomic_default_t>&         dest_rptr
    ,   const atomic_default_t                      value
    ,   const mgbase::operation&                    on_complete
    ) {
        const typename am_write::argument_type arg = { to_raw_pointer(dest_rptr), value };
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote write.\t"
            "dest_proc:{}\tdest_rptr:{:x}\tvalue:{}"
        ,   dest_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
        ,   arg.value
        );
        
        return mgcom::rpc::try_remote_call_async<am_write>(
            dest_proc
        ,   arg
        ,   on_complete
        );
    }
    
private:
    class am_compare_and_swap
    {
    public:
        static const mgcom::rpc::handler_id_t handler_id = 2004;
        
        struct argument_type {
            T*  target;
            T   expected;
            T   desired;
        };
        typedef T    return_type;
        
        static return_type on_request(
            const mgcom::rpc::handler_parameters& /*params*/
        ,   const argument_type& arg
        ) {
            // TODO
            mgbase::atomic<T>* const target = reinterpret_cast<mgbase::atomic<T>*>(arg.target);
            T expected = arg.expected;
            
            target->compare_exchange_strong(expected, arg.desired);
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote compare and swap.\t"
                "target_addr:{:x}\texpected:{}\tdesired:{}\tresult:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(arg.target)
            ,   arg.expected
            ,   arg.desired
            ,   expected
            );
            
            // Return the old value.
            return expected;
        }
    };
    
public:
    static bool try_compare_and_swap(
        const process_id_t                          target_proc
    ,   const remote_ptr<atomic_default_t>&         target_rptr
    ,   const atomic_default_t                      expected_val
    ,   const atomic_default_t                      desired_val
    ,   atomic_default_t* const                     result_ptr
    ,   const mgbase::operation&                    on_complete
    ) {
        const typename am_compare_and_swap::argument_type arg = {
            to_raw_pointer(target_rptr)
        ,   expected_val
        ,   desired_val
        };
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote compare and swap.\t"
            "proc:{:x}\ttarget:{:x}\texpected:{}\tdesired:{}\tresult_ptr:{:x}"
        ,   target_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(arg.target)
        ,   arg.expected
        ,   arg.desired
        ,   reinterpret_cast<mgbase::uintptr_t>(result_ptr)
        );
        
        return mgcom::rpc::try_remote_call_async<am_compare_and_swap>(
            target_proc
        ,   arg
        ,   result_ptr
        ,   on_complete
        );
    }
    
private:
    class am_fetch_and_add
    {
    public:
        static const mgcom::rpc::handler_id_t handler_id = 2006;
        
        struct argument_type {
            T*   target;
            T    diff;
        };
        typedef T    return_type;
        
        static return_type on_request(
            const mgcom::rpc::handler_parameters& /*params*/
        ,   const argument_type& arg
        ) {
            mgbase::atomic<T>* const target = reinterpret_cast<mgbase::atomic<T>*>(arg.target);
            
            // Returns the old value.
            const T result = target->fetch_add(arg.diff);
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote fetch and add.\t"
                "target_addr:{:x}\tdiff:{}\tresult:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(arg.target)
            ,   arg.diff
            ,   result
            );
            
            return result;
        }
    };
    
public:
    static bool try_fetch_and_add(
        const process_id_t                          target_proc
    ,   const remote_ptr<atomic_default_t>&         target_rptr
    ,   const atomic_default_t                      value
    ,   atomic_default_t* const                     result_ptr
    ,   const mgbase::operation&                    on_complete
    ) {
        const typename am_fetch_and_add::argument_type arg = {
            to_raw_pointer(target_rptr)
        ,   value
        };
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote fetch and add.\t"
            "proc:{:x}\ttarget_addr:{:x}\tdiff:{}\tresult_ptr:{:x}"
        ,   target_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(arg.target)
        ,   arg.diff
        ,   reinterpret_cast<mgbase::uintptr_t>(result_ptr)
        );
        
        return mgcom::rpc::try_remote_call_async<am_fetch_and_add>(
            target_proc
        ,   arg
        ,   result_ptr
        ,   on_complete
        );
    }
};

} // unnamed namespace

} // namespace rma
} // namespace mgcom

