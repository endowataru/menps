
#pragma once

#include <menps/mecom2/rma/basic_rma_ptr.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_public_rma_ptr
    : public basic_rma_ptr<P>
{
    using base = basic_rma_ptr<P>;
    
public:
    using element_type = typename P::element_type;
    
    using base::base;
    
    element_type* get() const noexcept {
        return this->get_ptr();
    }
    /*implicit*/ operator element_type*() const noexcept {
        return this->get_ptr();
    }
    
private:
    
};

} // namespace mecom2
} // namespace menps

