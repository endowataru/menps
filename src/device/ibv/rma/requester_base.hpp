
#pragma once

#include <mgcom/rma/requester.hpp>
#include "device/ibv/command/params.hpp"
#include "copy_params_to.hpp"

namespace mgcom {
namespace rma {

template <typename Derived>
class ibv_requester_base
    : public virtual rma::requester
{
private:
    struct read_closure
    {
        #if 0
        mgbase::uint64_t            wr_id;
        #endif
        const untyped::read_params& src_params;
        
        void operator() (ibv::read_command* const dest) const
        {
            #if 0
            dest_params->wr_id = wr_id;
            #endif
            
            ibv::copy_read_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_read_async(const untyped::read_params& params) MGBASE_OVERRIDE
    {
        #if 0
        auto& completer = derived().get_completer();
        
        mgbase::uint64_t wr_id = 0;
        if (MGBASE_UNLIKELY(
            !completer.try_complete(&wr_id)
        )) {
            return false;
        }
        
        completer.set_on_complete(wr_id, params.on_complete);
        #endif
        
        const bool ret = derived().template try_enqueue<ibv::read_command>(
            params.src_proc
        ,   Derived::command_code_type::ibv_read
        ,   read_closure{ /*wr_id,*/ params }
        );
        
        return ret;
        /*if (MGBASE_LIKELY(ret))
            return true;
        else {
            completer.failed(wr_id);
            return false;
        }*/
    }
    
private:
    struct write_closure
    {
        #if 0
        mgbase::uint64_t                wr_id;
        #endif
        const untyped::write_params&    src_params;
        
        void operator() (ibv::write_command* const dest) const
        {
            #if 0
            dest_params->wr_id = wr_id;
            #endif
            
            ibv::copy_write_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    bool try_write_async(const untyped::write_params& params) MGBASE_OVERRIDE
    {
        #if 0
        auto& completer = derived().get_completer();
        
        mgbase::uint64_t wr_id = 0;
        if (MGBASE_UNLIKELY(
            !completer.try_complete(&wr_id)
        )) {
            return false;
        }
        
        completer.set_on_complete(wr_id, params.on_complete);
        #endif
        
        const bool ret = derived().template try_enqueue<ibv::write_command>(
            params.dest_proc
        ,   Derived::command_code_type::ibv_write
        ,   write_closure{ params }
        );
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            #if 0
            completer.failed(wr_id);
            #endif
            return false;
        }
    }
    
private:
    struct atomic_read_closure
    {
        #if 0
        mgbase::uint64_t                            wr_id;
        const local_ptr<atomic_default_t>&          buf_lptr;
        #endif
        const atomic_read_params<atomic_default_t>& src_params;
        
        void operator() (ibv::atomic_read_command* const dest) const
        {
            #if 0
            dest_params->wr_id = wr_id;
            #endif
            
            ibv::copy_atomic_read_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    bool try_atomic_read_async(const atomic_read_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        #if 0
        auto& completer = derived().get_completer();
        
        mgbase::uint64_t wr_id = 0;
        if (MGBASE_UNLIKELY(
            !completer.try_complete(&wr_id)
        )) {
            return false;
        }
        
        auto& atomic_buf = derived().get_atomic_buffer();
        
        const auto r = atomic_buf.make_notification_read(wr_id, params.on_complete, params.dest_ptr);
        completer.set_on_complete(wr_id, r.on_complete);
        #endif
        
        const bool ret = derived().template try_enqueue<ibv::atomic_read_command>(
            params.src_proc
        ,   Derived::command_code_type::ibv_read
        ,   atomic_read_closure{ params }
        #if 0
        ,   atomic_read_closure{ wr_id, r.buf_lptr, params }
        #endif
        );
        
        if (MGBASE_LIKELY(ret)) {
            return true;
        }
        else {
            #if 0
            completer.failed(wr_id);
            #endif
            return false;
        }
    }
    
private:
    struct atomic_write_closure
    {
        #if 0
        mgbase::uint64_t                                wr_id;
        const local_ptr<atomic_default_t>&              buf_lptr;
        #endif
        const atomic_write_params<atomic_default_t>&    src_params;
        
        void operator() (ibv::atomic_write_command* const dest) const
        {
            #if 0
            dest_params->wr_id = wr_id;
            #endif
            
            ibv::copy_atomic_write_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    bool try_atomic_write_async(const atomic_write_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        #if 0
        auto& completer = derived().get_completer();
        
        mgbase::uint64_t wr_id = 0;
        if (MGBASE_UNLIKELY(
            !completer.try_complete(&wr_id)
        )) {
            return false;
        }
        
        auto& atomic_buf = derived().get_atomic_buffer();
        
        const auto r = atomic_buf.make_notification_write(wr_id, params.on_complete);
        completer.set_on_complete(wr_id, r.on_complete);
        
        // Assign a value to the buffer.
        *r.buf_lptr = params.value;
        #endif
        
        const bool ret = derived().template try_enqueue<ibv::atomic_write_command>(
            params.dest_proc
        ,   Derived::command_code_type::ibv_write
        ,   atomic_write_closure{ params }
        //,   atomic_write_closure{ wr_id, r.buf_lptr, params }
        );
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            #if 0
            completer.failed(wr_id);
            #endif
            return false;
        }
    }
    
private:
    struct compare_and_swap_closure
    {
        #if 0
        mgbase::uint64_t                                    wr_id;
        const local_ptr<atomic_default_t>&                  buf_lptr;
        #endif
        const compare_and_swap_params<atomic_default_t>&    src_params;
        
        void operator() (ibv::compare_and_swap_command* const dest) const
        {
            #if 0
            dest_params->wr_id = wr_id;
            #endif
            
            ibv::copy_compare_and_swap_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    bool try_compare_and_swap_async(const compare_and_swap_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        #if 0
        auto& completer = derived().get_completer();
        
        mgbase::uint64_t wr_id = 0;
        if (MGBASE_UNLIKELY(
            !completer.try_complete(&wr_id)
        )) {
            return false;
        }
        
        auto& atomic_buf = derived().get_atomic_buffer();
        
        const auto r = atomic_buf.make_notification_atomic(wr_id, params.on_complete, params.result_ptr);
        completer.set_on_complete(wr_id, r.on_complete);
        #endif
        
        const bool ret = derived().template try_enqueue<ibv::compare_and_swap_command>(
            params.target_proc
        ,   Derived::command_code_type::ibv_compare_and_swap
        ,   compare_and_swap_closure{ params }
        //,   compare_and_swap_closure{ wr_id, r.buf_lptr, params }
        );
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            #if 0
            completer.failed(wr_id);
            #endif
            return false;
        }
    }
    
private:
    struct fetch_and_add_closure
    {
        #if 0
        mgbase::uint64_t                                    wr_id;
        const local_ptr<atomic_default_t>&                  buf_lptr;
        #endif
        const fetch_and_add_params<atomic_default_t>&       src_params;
        
        void operator() (ibv::fetch_and_add_command* const dest) const
        {
            #if 0
            dest_params->wr_id = wr_id;
            #endif
            
            ibv::copy_fetch_and_add_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    bool try_fetch_and_add_async(const fetch_and_add_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        #if 0
        auto& completer = derived().get_completer();
        
        mgbase::uint64_t wr_id = 0;
        if (MGBASE_UNLIKELY(
            !completer.try_complete(&wr_id)
        )) {
            return false;
        }
        
        auto& atomic_buf = derived().get_atomic_buffer();
        
        const auto r = atomic_buf.make_notification_atomic(wr_id, params.on_complete, params.result_ptr);
        completer.set_on_complete(wr_id, r.on_complete);
        #endif
        
        const bool ret = derived().template try_enqueue<ibv::fetch_and_add_command>(
            params.target_proc
        ,   Derived::command_code_type::ibv_fetch_and_add
        ,   fetch_and_add_closure{ params }
        );
        
        if (MGBASE_LIKELY(ret))
            return true;
        else {
            #if 0
            completer.failed(wr_id);
            #endif
            return false;
        }
    }
    
private:
    Derived& derived() MGBASE_NOEXCEPT {
        return static_cast<Derived&>(*this);
    }
};

} // namespace rma
} // namespace mgcom

