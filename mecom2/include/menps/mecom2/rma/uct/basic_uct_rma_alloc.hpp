
#pragma once

#include <menps/mecom2/rma/rma_typed_allocator.hpp>
#include <menps/mecom2/rma/rma_private_heap_alloc.hpp>
#include <menps/mefdn/memory/distance_in_bytes.hpp>
#include <cstring>

namespace menps {
namespace mecom2 {

//#define MECOM2_RMA_UCT_ALLOCATE
#define MECOM2_RMA_USE_DLMALLOC_ALLOCATOR

template <typename P>
class basic_uct_rma_alloc
    : public rma_typed_allocator<P>
    , public rma_private_heap_alloc<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using uct_itf_type = typename P::uct_itf_type;
    using memory_type = typename uct_itf_type::memory_type;
    using remote_key_type = typename uct_itf_type::remote_key_type;
    
    using public_minfo_type = typename P::public_minfo_type;
    using remote_minfo_type = typename P::remote_minfo_type;
    
    template <typename T>
    using remote_ptr_t = typename P::template remote_ptr<T>;
    template <typename T>
    using public_ptr_t = typename P::template public_ptr<T>;
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
public:
    #ifndef MECOM2_RMA_USE_DLMALLOC_ALLOCATOR
    public_ptr_t<void> untyped_allocate(const size_type size_in_bytes)
    {
        #ifdef MECOM2_RMA_UCT_ALLOCATE
            auto& self = this->derived();
            auto& uf = self.get_uct_facade();
            auto& md = self.get_md();
            
            // TODO: This returns an error on InfiniBand by default.
            auto mem =
                memory_type::mem_alloc(
                    uf, md.get(), size_in_bytes,
                    nullptr, UCT_MD_MEM_ACCESS_ALL, "mecom2"
                );
            
            return this->make_public_minfo(mefdn::move(mem));
        #else
            const auto buf = new mefdn::byte[size_in_bytes];
            return this->attach(buf, buf+size_in_bytes);
        #endif
    }
    void untyped_deallocate(const public_ptr_t<void>& lptr) {
        this->destroy_minfo(lptr);
        #ifndef MECOM2_RMA_UCT_ALLOCATE
            delete[] static_cast<mefdn::byte*>(lptr.get());
        #endif
    }
    #endif
    
    template <typename T>
    public_ptr_t<T> attach(T* const first, T* const last)
    {
        auto& self = this->derived();
        auto& uf = self.get_uct_facade();
        auto& md = self.get_md();
        
        const auto size_in_bytes =
            mefdn::distance_in_bytes(first, last);
        
        auto mem =
            memory_type::mem_reg(
                uf, md.get(), first, size_in_bytes, 
                UCT_MD_MEM_FLAG_FIXED | UCT_MD_MEM_ACCESS_ALL
            );
        
        return this->make_public_minfo(mefdn::move(mem))
            .template static_cast_to<T>();
    }
    void detach(const public_ptr_t<void>& lptr) {
        this->destroy_minfo(lptr);
    }
    
private:
    public_ptr_t<void> make_public_minfo(memory_type mem)
    {
        auto& self = this->derived();
        const auto& md_attr = self.get_md_attr();
        
        auto rkey_buf =
            uct_itf_type::pack_rkey(mem, md_attr);
        
        const auto addr = mem.get_address();
        const auto minfo_ptr =
            new public_minfo_type{ mefdn::move(mem), mefdn::move(rkey_buf) };
        
        return public_ptr_t<void>(addr, minfo_ptr);
    }
    
    void destroy_minfo(const public_ptr_t<void>& lptr) {
        delete lptr.get_minfo();
    }
    
public:
    size_type serialized_size_in_bytes(const public_ptr_t<void>& lptr)
    {
        const auto* minfo = lptr.get_minfo();
        
        return sizeof(void*) + minfo->rkey_buf.size_in_bytes();
    }
    void serialize(const public_ptr_t<void>& lptr, void* const buf)
    {
        const auto ptr_buf = static_cast<void**>(buf);
        // Place a pointer at the beginning of the buffer.
        *ptr_buf = lptr.get();
        
        const auto* minfo = lptr.get_minfo();
        
        const auto size = minfo->rkey_buf.size_in_bytes();
        // Copy the RKEY to the rest of the buffer.
        std::memcpy(ptr_buf+1, minfo->rkey_buf.get(), size);
    }
    template <typename T>
    remote_ptr_t<T> deserialize(const proc_id_type /*proc*/, const void* const buf)
    {
        auto& self = this->derived();
        auto& uf = self.get_uct_facade();
        
        const auto ptr_buf = static_cast<void* const *>(buf);
        // Load the pointer from the buffer.
        const auto raddr = *ptr_buf;
        
        auto rkey = remote_key_type::unpack(uf, ptr_buf+1);
        
        // TODO: Leaking memory.
        const auto minfo_ptr =
            new remote_minfo_type{ mefdn::move(rkey) };
        
        return remote_ptr_t<T>(static_cast<T*>(raddr), minfo_ptr);
    }
};

} // namespace mecom2
} // namespace menps

