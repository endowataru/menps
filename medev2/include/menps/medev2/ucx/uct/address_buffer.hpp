
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

struct address_buffer_deleter
{
    void operator () (void* const p) const noexcept
    {
        delete [] static_cast<mefdn::byte*>(p);
    }
};

template <typename Addr>
class address_buffer
    : public mefdn::unique_ptr<Addr, address_buffer_deleter>
{
    using deleter_type = address_buffer_deleter;
    using base = mefdn::unique_ptr<Addr, deleter_type>;
    
public:
    address_buffer() noexcept = default;
    
    explicit address_buffer(
        Addr* const             p
    ,   const mefdn::size_t     size
    )
        : base(p)
        , size_(size)
    { }
    
    address_buffer(const address_buffer&) = delete;
    address_buffer& operator = (const address_buffer&) = delete;
    
    address_buffer(address_buffer&&) noexcept = default;
    address_buffer& operator = (address_buffer&&) noexcept = default;
    
    mefdn::size_t size_in_bytes() const noexcept {
        return this->size_;
    }
    
    static address_buffer make(const mefdn::size_t size)
    {
        const auto ptr =
            reinterpret_cast<Addr*>(
                // Note: default initialization by parenthesis
                new mefdn::byte[size]()
            );
        
        return address_buffer(ptr, size);
    }
    
private:
    mefdn::size_t size_ = 0;
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

