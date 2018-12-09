
#pragma once

#include <menps/mecom/rma/requester.hpp>

namespace menps {
namespace mecom {
namespace rma {

template <typename Derived>
class fjmpi_requester_base
    : public virtual rma::requester
{
public:
    MEFDN_NODISCARD
    virtual bool try_read_async(const untyped::read_params& params) MEFDN_OVERRIDE
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
    
    MEFDN_NODISCARD
    virtual bool try_write_async(const untyped::write_params& params) MEFDN_OVERRIDE
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
    Derived& derived() noexcept {
        return static_cast<Derived&>(*this);
    }
};

} // namespace rma
} // namespace mecom
} // namespace menps

