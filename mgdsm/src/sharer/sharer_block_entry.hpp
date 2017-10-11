
#pragma once

#include "sharer_block_state.hpp"
#include <mgcom/rma/unique_local_ptr.hpp>

namespace mgdsm {

class sharer_block_entry
    : public sharer_block_state
{
public:
    sharer_block_entry()
        : twin_{}
    { }
    
    mgcom::rma::local_ptr<void> get_twin_lptr(const mgbase::size_t blk_size) MGBASE_NOEXCEPT
    {
        if (! twin_) {
            twin_.reset(mgcom::rma::local_ptr<void>::cast_from(mgcom::rma::untyped::to_address(mgcom::rma::untyped::allocate(blk_size))));
                // TODO: very ugly...
        }
        
        return twin_.get();
    }
    
private:
    mgcom::rma::unique_local_ptr<void> twin_;
};

} // namespace mgdsm

