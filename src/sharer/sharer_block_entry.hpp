
#pragma once

#include "sharer_block_state.hpp"
#include <mgcom/rma/unique_local_ptr.hpp>

namespace mgdsm {

class sharer_block_entry
    : public sharer_block_state
{
public:
    mgcom::rma::local_ptr<void> get_twin_lptr() const MGBASE_NOEXCEPT {
        return twin_.get();
    }
    
private:
    mgcom::rma::unique_local_ptr<void> twin_;
};

} // namespace mgdsm

