
#pragma once

#include <menps/medev/ucx/ucp/remote_key.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

struct rkey_buffer_deleter
{
    void operator() (void*) const noexcept;
};

class rkey_buffer
    : public mefdn::unique_ptr<void, rkey_buffer_deleter>
{
    typedef mefdn::unique_ptr<void, rkey_buffer_deleter>  base;
    
public:
    rkey_buffer() noexcept = default;
    
    explicit rkey_buffer(void* const p, const mefdn::size_t size)
        : base(p)
        , size_(size)
    { }
    
    rkey_buffer(const rkey_buffer&) = delete;
    rkey_buffer& operator = (const rkey_buffer&) = delete;
    
    rkey_buffer(rkey_buffer&&) noexcept = default;
    rkey_buffer& operator = (rkey_buffer&&) noexcept = default;
    
    mefdn::size_t size() const noexcept {
        return size_;
    }
    
    remote_key unpack(ucp_ep* ep);
    
private:
    mefdn::size_t size_;
};

rkey_buffer pack_rkey(ucp_context* ctx, ucp_mem* mem);

} // namespace ucp
} // namesapce ucx
} // namespace medev
} // namespace menps

