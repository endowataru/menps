
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
        const untyped::read_params& src_params;
        
        void operator() (ibv::read_command* const dest) const
        {
            ibv::copy_read_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_read(
        const untyped::read_params& params
    ) MGBASE_OVERRIDE 
    {
        while (! derived().template try_enqueue<ibv::read_command>(
            params.src_proc
        ,   Derived::command_code_type::ibv_read
        ,   read_closure{ /*wr_id,*/ params }
        )) {
            ult::yield();
        }
        
        return ult::make_async_deferred<void>();
    }
    
private:
    struct write_closure
    {
        const untyped::write_params&    src_params;
        
        void operator() (ibv::write_command* const dest) const
        {
            ibv::copy_write_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_write(
        const untyped::write_params& params
    ) MGBASE_OVERRIDE
    {
        while (! derived().template try_enqueue<ibv::write_command>(
            params.dest_proc
        ,   Derived::command_code_type::ibv_write
        ,   write_closure{ params }
        )) {
            ult::yield();
        }
        
        return ult::make_async_deferred<void>();
    }
    
private:
    struct atomic_read_closure
    {
        const async_atomic_read_params<atomic_default_t>& src_params;
        
        void operator() (ibv::atomic_read_command* const dest) const
        {
            ibv::copy_atomic_read_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_read(
        const async_atomic_read_params<atomic_default_t>& params
    ) MGBASE_OVERRIDE
    {
        while (! derived().template try_enqueue<ibv::atomic_read_command>(
            params.src_proc
        ,   Derived::command_code_type::ibv_atomic_read
        ,   atomic_read_closure{ params }
        )) {
            ult::yield();
        }
        
        return ult::make_async_deferred<void>();
    }
    
private:
    struct atomic_write_closure
    {
        const async_atomic_write_params<atomic_default_t>&    src_params;
        
        void operator() (ibv::atomic_write_command* const dest) const
        {
            ibv::copy_atomic_write_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_write(
        const async_atomic_write_params<atomic_default_t>& params
    ) MGBASE_OVERRIDE 
    {
        while (! derived().template try_enqueue<ibv::atomic_write_command>(
            params.dest_proc
        ,   Derived::command_code_type::ibv_atomic_write
        ,   atomic_write_closure{ params }
        )) {
            ult::yield();
        }
        
        return ult::make_async_deferred<void>();
    }
    
private:
    struct compare_and_swap_closure
    {
        const async_compare_and_swap_params<atomic_default_t>&    src_params;
        
        void operator() (ibv::compare_and_swap_command* const dest) const
        {
            ibv::copy_compare_and_swap_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_compare_and_swap(
        const async_compare_and_swap_params<atomic_default_t>& params
    ) MGBASE_OVERRIDE
    {
        while (! derived().template try_enqueue<ibv::compare_and_swap_command>(
            params.target_proc
        ,   Derived::command_code_type::ibv_compare_and_swap
        ,   compare_and_swap_closure{ params }
        )) {
            ult::yield();
        }
        
        return ult::make_async_deferred<void>();
    }
    
private:
    struct fetch_and_add_closure
    {
        const async_fetch_and_add_params<atomic_default_t>&       src_params;
        
        void operator() (ibv::fetch_and_add_command* const dest) const
        {
            ibv::copy_fetch_and_add_params_to(src_params, dest);
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_fetch_and_add(
        const async_fetch_and_add_params<atomic_default_t>& params
    ) MGBASE_OVERRIDE
    {
        while (! derived().template try_enqueue<ibv::fetch_and_add_command>(
            params.target_proc
        ,   Derived::command_code_type::ibv_fetch_and_add
        ,   fetch_and_add_closure{ params }
        )) {
            ult::yield();
        }
        
        return ult::make_async_deferred<void>();
    }
    
private:
    Derived& derived() MGBASE_NOEXCEPT {
        return static_cast<Derived&>(*this);
    }
};

} // namespace rma
} // namespace mgcom

