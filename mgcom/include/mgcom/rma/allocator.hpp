
#pragma once

#include <mgcom/rma/address.hpp>

namespace mgcom {
namespace rma {

class allocator
{
public:
    virtual ~allocator() MGBASE_EMPTY_DEFINITION
    
    MGBASE_WARN_UNUSED_RESULT
    virtual untyped::registered_buffer allocate(index_t size_in_bytes) = 0;
    
    virtual void deallocate(const untyped::registered_buffer& buffer) = 0;
    
    static allocator& get_instance() MGBASE_NOEXCEPT {
        return *alloc_;
    }
    
    static void set_instance(allocator& alloc) {
        alloc_ = &alloc;
    }
    
protected:
    allocator() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
private:
    static allocator* alloc_;
};

} // namespace rma
} // namespace mgcom

