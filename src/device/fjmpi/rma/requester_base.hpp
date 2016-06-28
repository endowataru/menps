
#pragma once

#include <mgcom/rma/requester.hpp>

namespace mgcom {
namespace rma {

template <typename Derived>
class fjmpi_requester_base
    : public virtual rma::requester
{
public:
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_read_async(const untyped::read_params& params) MGBASE_OVERRIDE
    {
        struct closure
        {
            const untyped::read_params&     params;
            Derived&                        self;
            
            void operator() (fjmpi::get_params* const dest) const
            {
                *dest = {
                    static_cast<int>(params.src_proc)
                ,   fjmpi::get_absolute_address(params.dest_laddr) // Note: Be careful of the order
                ,   fjmpi::get_absolute_address(params.src_raddr)
                ,   params.size_in_bytes
                ,   self.select_flags()
                ,   params.on_complete
                };
            }
        };
        
        return derived().template try_enqueue<fjmpi::get_params>(
            Derived::command_code_type::fjmpi_get
        ,   closure{ params, derived() }
        );
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_write_async(const untyped::write_params& params) MGBASE_OVERRIDE
    {
        struct closure
        {
            const untyped::write_params&    params;
            Derived&                        self;
            
            void operator() (fjmpi::put_params* const dest) const
            {
                *dest = {
                    static_cast<int>(params.dest_proc)
                ,   fjmpi::get_absolute_address(params.src_laddr) // Note: Be careful of the order
                ,   fjmpi::get_absolute_address(params.dest_raddr)
                ,   params.size_in_bytes
                ,   self.select_flags()
                ,   params.on_complete
                };
            }
        };
        
        return derived().template try_enqueue<fjmpi::put_params>(
            Derived::command_code_type::fjmpi_put
        ,   closure{ params, derived() }
        );
    }
    
private:
    Derived& derived() MGBASE_NOEXCEPT {
        return static_cast<Derived&>(*this);
    }
};

} // namespace rma
} // namespace mgcom

