
#pragma once

#include "sharer_block_state.hpp"
#include <menps/mecom/rma/unique_local_ptr.hpp>

namespace menps {
namespace medsm {

class sharer_block_entry
    : public sharer_block_state
{
public:
    sharer_block_entry()
        : twin_{}
    { }
    
    mecom::rma::local_ptr<void> get_twin_lptr(const mefdn::size_t blk_size) noexcept
    {
        if (! twin_) {
            twin_.reset(mecom::rma::local_ptr<void>::cast_from(mecom::rma::untyped::to_address(mecom::rma::untyped::allocate(blk_size))));
                // TODO: very ugly...
        }
        
        return twin_.get();
    }
    
private:
    mecom::rma::unique_local_ptr<void> twin_;
};

} // namespace medsm
} // namespace menps

