
#pragma once

#include <menps/mecom2/rma/basic_rma_ptr.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_remote_rma_ptr
    : public basic_rma_ptr<P>
{
    using base = basic_rma_ptr<P>;
    
    using remote_addr_type = typename P::remote_addr_type;
    
public:
    using element_type = typename P::element_type;
    
    using base::base;
    
    remote_addr_type get_addr() const noexcept {
        return reinterpret_cast<remote_addr_type>(this->get_ptr());
    }
    
private:
    
};

} // namespace mecom2
} // namespace menps

