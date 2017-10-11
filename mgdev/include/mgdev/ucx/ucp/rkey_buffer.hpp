
#pragma once

#include <mgdev/ucx/ucp/remote_key.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

struct rkey_buffer_deleter
{
    void operator() (void*) const MGBASE_NOEXCEPT;
};

class rkey_buffer
    : public mgbase::unique_ptr<void, rkey_buffer_deleter>
{
    typedef mgbase::unique_ptr<void, rkey_buffer_deleter>  base;
    
public:
    rkey_buffer() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit rkey_buffer(void* const p, const mgbase::size_t size)
        : base(p)
        , size_(size)
    { }
    
    rkey_buffer(const rkey_buffer&) = delete;
    rkey_buffer& operator = (const rkey_buffer&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_1(rkey_buffer, base, size_)
    
    mgbase::size_t size() const MGBASE_NOEXCEPT {
        return size_;
    }
    
    remote_key unpack(ucp_ep* ep);
    
private:
    mgbase::size_t size_;
};

rkey_buffer pack_rkey(ucp_context* ctx, ucp_mem* mem);

} // namespace ucp
} // namesapce ucx
} // namespace mgdev

