
#pragma once

#include <menps/mecom/rma/address.hpp>

namespace menps {
namespace mecom {
namespace rma {

class allocator
{
public:
    virtual ~allocator() /*noexcept*/ = default;
    
    MEFDN_NODISCARD
    virtual untyped::registered_buffer allocate(index_t size_in_bytes) = 0;
    
    virtual void deallocate(const untyped::registered_buffer& buffer) = 0;
    
    static allocator& get_instance() noexcept {
        return *alloc_;
    }
    
    static void set_instance(allocator& alloc) {
        alloc_ = &alloc;
    }
    
protected:
    allocator() noexcept = default;
    
private:
    static allocator* alloc_;
};

} // namespace rma
} // namespace mecom
} // namespace menps

