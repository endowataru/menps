
#pragma once

#include <menps/medev2/ucx/ucp/ucp.hpp>
#include <menps/medev2/ucx/ucx_error.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

template <typename P>
struct packed_rkey_deleter
{
    using ucp_facade_type = typename P::ucp_facade_type;
    
    ucp_facade_type* uf;
    
    void operator() (void* const p) const noexcept {
        uf->rkey_buffer_release({ p });
    }
};

template <typename P>
class packed_rkey
    : public mefdn::unique_ptr<void, packed_rkey_deleter<P>>
{
    using deleter_type = packed_rkey_deleter<P>;
    using base = mefdn::unique_ptr<void, deleter_type>;
    
    using ucp_facade_type = typename P::ucp_facade_type;
    
public:
    packed_rkey() noexcept = default;
    
    explicit packed_rkey(
        ucp_facade_type&    uf
    ,   void* const         p
    ,   const mefdn::size_t size
    )
        : base(p, deleter_type{ &uf })
        , size_(size)
    { }
    
    packed_rkey(const packed_rkey&) = delete;
    packed_rkey& operator = (const packed_rkey&) = delete;
    
    packed_rkey(packed_rkey&&) noexcept = default;
    packed_rkey& operator = (packed_rkey&&) noexcept = default;
    
    mefdn::size_t size_in_bytes() const noexcept {
        return size_;
    }
    
    static packed_rkey pack(
        ucp_facade_type&    uf
    ,   ucp_context* const  ctx
    ,   ucp_mem* const      mem
    ) {
        void* p = nullptr;
        mefdn::size_t size = 0;
        
        const auto ret = uf.rkey_pack({ ctx, mem, &p, &size });
        if (ret != UCS_OK) {
            throw ucx_error("ucp_rkey_pack() failed", ret);
        }
        
        return packed_rkey(uf, p, size);
    }
    
private:
    mefdn::size_t size_ = 0;
};

} // namespace ucp
} // namesapce ucx
} // namespace medev2
} // namespace menps

