
#pragma once

#include <mgcom/am.hpp>
#include <mgcom/rma.hpp>
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
        mgcom::am::register_roundtrip_handler<am_read>();
        mgcom::am::register_roundtrip_handler<am_write>();
        mgcom::am::register_roundtrip_handler<am_fetch_and_add>();
        mgcom::am::register_roundtrip_handler<am_compare_and_swap>();
    }

private:
    class am_read
    {
    public:
        static const mgcom::am::handler_id_t request_id = 2000; // TODO: remove magic numbers
        static const mgcom::am::handler_id_t reply_id   = 2001;
        
        struct argument_type {
            const T* ptr;
        };
        typedef T    return_type;
        
        static return_type on_request(
            const mgcom::am::callback_parameters& /*params*/
        ,   const argument_type& arg
        ) {
            const T result = mgbase::atomic_load(arg.ptr);
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote write.\t"
                "src_ptr:{:x}\tresult:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
            ,   result
            );
            
            return result;
        }
    };
    
public:
    template <typename CB>
    static mgbase::deferred<void> read(CB& cb)
    {
        const T* const src_ptr
            = static_cast<const T*>(mgcom::rma::untyped::to_raw_pointer(cb.local_addr));
        
        const typename am_read::argument_type arg = { src_ptr };
        
        T* const dest_ptr
            = static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(cb.remote_addr));
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote read.\t"
            "proc:{}\tsrc_ptr:{:x}"
        ,   cb.proc
        ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
        );
        
        return mgcom::am::call_roundtrip_nb<am_read>(
            cb.cb_roundtrip
        ,   cb.proc
        ,   arg
        ,   dest_ptr
        );
    }
    
private:
    class am_write
    {
    public:
        static const mgcom::am::handler_id_t request_id = 2002;
        static const mgcom::am::handler_id_t reply_id   = 2003;
        
        struct argument_type {
            T*   ptr;
            T    value;
        };
        typedef void    return_type;
        
        static return_type on_request(
            const mgcom::am::callback_parameters& /*params*/
        ,   const argument_type& arg
        ) {
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote write.\t"
                "value:{}\tdest_ptr:{:x}"
            ,   arg.value
            ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
            );
            
            mgbase::atomic_store(arg.ptr, arg.value);
        }
    };
    
public:
    template <typename CB>
    static mgbase::deferred<void> write(CB& cb)
    {
        const T* const src_ptr
            = static_cast<const T*>(mgcom::rma::untyped::to_raw_pointer(cb.local_addr));
        
        T* const dest_ptr
            = static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(cb.remote_addr));
        
        const typename am_write::argument_type arg = { dest_ptr, *src_ptr };
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote write.\t"
            "proc:{}\tvalue:{}\tdest_ptr:{:x}"
        ,   cb.proc
        ,   arg.value
        ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
        );
        
        return mgcom::am::call_roundtrip_nb<am_write>(
            cb.cb_roundtrip
        ,   cb.proc
        ,   arg
        );
    }
    
private:
    class am_compare_and_swap
    {
    public:
        static const mgcom::am::handler_id_t request_id = 2004;
        static const mgcom::am::handler_id_t reply_id   = 2005;
        
        struct argument_type {
            T*  target;
            T   expected;
            T   desired;
        };
        typedef T    return_type;
        
        static return_type on_request(
            const mgcom::am::callback_parameters& /*params*/
        ,   const argument_type& arg
        ) {
            T expected = arg.expected;
            mgbase::atomic_compare_exchange_strong(arg.target, &expected, arg.desired);
            
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
    template <typename CB>
    static mgbase::deferred<void> compare_and_swap(CB& cb, process_id_t target_proc)
    {
        T* const target_ptr
            = static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(cb.target_addr));
        
        const T* const expected_ptr
            = static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(cb.expected_addr));
        
        const T* const desired_ptr
            = static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(cb.desired_addr));
        
        const typename am_compare_and_swap::argument_type arg = { target_ptr, *expected_ptr, *desired_ptr };
        
        T* const result_ptr
            = static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(cb.result_addr));
         
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote compare and swap.\t"
            "proc:{:x}\ttarget_addr:{:x}\texpected:{}\tdesired:{}\tresult_ptr:{:x}"
        ,   target_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(target_ptr)
        ,   arg.expected
        ,   arg.desired
        ,   reinterpret_cast<mgbase::uintptr_t>(result_ptr)
        );

        return mgcom::am::call_roundtrip_nb<am_compare_and_swap>(
            cb.cb_roundtrip
        ,   target_proc
        ,   arg
        ,   result_ptr
        );
    }
    
private:
    class am_fetch_and_add
    {
    public:
        static const mgcom::am::handler_id_t request_id = 2006;
        static const mgcom::am::handler_id_t reply_id   = 2007;
        
        struct argument_type {
            T*   ptr;
            T    diff;
        };
        typedef T    return_type;
        
        static return_type on_request(
            const mgcom::am::callback_parameters& /*params*/
        ,   const argument_type& arg
        ) {
            // Returns the old value.
            const T result = mgbase::atomic_fetch_add(arg.ptr, arg.diff);
            
            MGBASE_LOG_DEBUG(
                "msg:Emulated remote fetch and add.\t"
                "target_addr:{:x}\tdiff:{}\tresult:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
            ,   arg.diff
            ,   result
            );
            
            return result;
        }
    };
    
public:
    template <typename CB>
    static mgbase::deferred<void> fetch_and_add(CB& cb, process_id_t target_proc)
    {
        T* const target_ptr
            = static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(cb.target_addr));
        
        const T* const value_ptr
            = static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(cb.value_addr));
        
        const typename am_fetch_and_add::argument_type arg = { target_ptr, *value_ptr };
        
        T* const result_ptr
            = static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(cb.result_addr));
        
        MGBASE_LOG_DEBUG(
            "msg:Send message of emulated remote fetch and add.\t"
            "proc:{:x}\ttarget_addr:{:x}\tdiff:{}\tresult_ptr:{:x}"
        ,   target_proc
        ,   reinterpret_cast<mgbase::uintptr_t>(arg.ptr)
        ,   arg.diff
        ,   reinterpret_cast<mgbase::uintptr_t>(result_ptr)
        );
        
        return mgcom::am::call_roundtrip_nb<am_fetch_and_add>(
            cb.cb_roundtrip
        ,   target_proc
        ,   arg
        ,   result_ptr
        );
    }
};

} // unnamed namespace

} // namespace rma
} // namespace mgcom

