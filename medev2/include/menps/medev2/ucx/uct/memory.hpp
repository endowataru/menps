
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

extern "C" {

// Explicitly define here for type safety.
struct uct_mem;

} // extern C

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct memory_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type*    uf;
    uct_md*             md;
    bool                is_reg;
    
    void operator () (uct_mem* const p) const noexcept
    {
        // TODO: This requires a dynamic dispatch,
        //       though the cost is small and in the destruction.
        if (is_reg) {
            this->uf->md_mem_dereg({ this->md, p });
        }
        else {
            this->uf->md_mem_free({ this->md, p });
        }
    }
};

template <typename P>
class memory
    : public mefdn::unique_ptr<uct_mem, memory_deleter<P>>
{
    using deleter_type = memory_deleter<P>;
    using base = mefdn::unique_ptr<uct_mem, deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    memory() noexcept = default;
    
    explicit memory(
        uct_facade_type&    uf
    ,   uct_md* const       md
    ,   uct_mem* const      mem
    ,   const bool          is_reg
    ,   void* const         ptr
    ,   const mefdn::size_t size
    )
        : base(mem, deleter_type{ &uf, md, is_reg })
        , ptr_(ptr)
        , size_(size)
    { }
    
    memory(const memory&) = delete;
    memory& operator = (const memory&) = delete;
    
    memory(memory&&) noexcept = default;
    memory& operator = (memory&&) noexcept = default;
    
    static memory mem_reg(
        uct_facade_type&        uf
    ,   const uct_md_h          md
    ,   void* const             address
    ,   const mefdn::size_t     length
    ,   const unsigned          flags
    )  {
        uct_mem_h memh = nullptr;
        
        const auto ret = uf.md_mem_reg({ md, address, length, flags, &memh });
        if (ret != UCS_OK) {
            throw ucx_error("uct_md_mem_reg() failed", ret);
        }
        
        return memory(uf, md, static_cast<uct_mem*>(memh),
            true, address, length);
    }
    
    static memory mem_alloc(
        uct_facade_type&    uf
    ,   const uct_md_h      md
    ,   /*mutable*/ size_t  length
    ,   void* /*mutable*/   address
    ,   const unsigned      flags
    ,   const char * const  name
    ) {
        uct_mem_h memh = nullptr;
        
        const auto ret =
            uf.md_mem_alloc({ md, &length, &address, flags, name, &memh });
        
        if (ret != UCS_OK) {
            throw ucx_error("uct_md_mem_alloc() failed", ret);
        }
        
        return memory(uf, md, static_cast<uct_mem*>(memh),
            false, address, length);
    }
    
    void* get_address() const noexcept {
        return this->ptr_;
    }
    mefdn::size_t get_size() const noexcept {
        return this->size_;
    }
    
    void pack_rkey(void* const rkey_buf) const
    {
        const auto& del = this->get_deleter();
        const auto uf = del.uf;
        const auto md = del.md;
        
        const auto ret =
            uf->md_mkey_pack({ md, this->get(), rkey_buf });
        
        if (ret != UCS_OK) {
            throw ucx_error("uct_md_mkey_pack() failed", ret);
        }
    }
    
private:
    void* ptr_ = nullptr;
    mefdn::size_t size_ = 0;
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

