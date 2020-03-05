
#pragma once

#include <menps/mecom2/rma/rma_typed_allocator.hpp>
#include <menps/mecom2/rma/rma_private_heap_alloc.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_ucp_rma_alloc
    : public rma_typed_allocator<P>
    , public rma_private_heap_alloc<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using ucp_itf_type = typename P::ucp_itf_type;
    using ucp_facade_type = typename ucp_itf_type::ucp_facade_type;
    using memory_type = typename ucp_itf_type::memory_type;
    using packed_rkey_type = typename ucp_itf_type::packed_rkey_type;
    using remote_key_type = typename ucp_itf_type::remote_key_type;
    
    using public_minfo_type = typename P::public_minfo_type;
    using remote_minfo_type = typename P::remote_minfo_type;
    
    template <typename T>
    using remote_ptr_t = typename P::template remote_ptr<T>;
    template <typename T>
    using public_ptr_t = typename P::template public_ptr<T>;
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    using worker_num_type = typename P::worker_num_type;
    
public:
    public_ptr_t<void> untyped_allocate(const mefdn::size_t size_in_bytes)
    {
        auto& self = this->derived();
        auto& uf = self.get_ucp_facade();
        auto& ctx = self.get_context();
        
        auto mem =
            memory_type::map_allocate(
                uf, ctx.get(), size_in_bytes);
        
        return this->make_public_minfo(mefdn::move(mem));
    }
    void untyped_deallocate(const public_ptr_t<void>& lptr) {
        this->destroy_minfo(lptr);
    }
    
    template <typename T>
    public_ptr_t<T> attach(T* const first, T* const last)
    {
        auto& self = this->derived();
        auto& uf = self.get_ucp_facade();
        auto& ctx = self.get_context();
        
        const auto size_in_bytes =
            mefdn::distance_in_bytes(first, last);
        
        auto mem =
            memory_type::map_register(
                uf, ctx.get(), first, size_in_bytes
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
        auto& uf = self.get_ucp_facade();
        auto& ctx = self.get_context();
        
        
        auto rkey_buf =
            packed_rkey_type::pack(
                uf, ctx.get(), mem.get()
            );
        
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
    remote_ptr_t<T> deserialize(const proc_id_type proc, const void* const buf)
    {
        auto& self = this->derived();
        auto& uf = self.get_ucp_facade();
        
        const auto ptr_buf = static_cast<void* const *>(buf);
        // Load the pointer from the buffer.
        const auto raddr = *ptr_buf;
        
        const auto num_wks = self.get_num_workers();
        auto rkeys = mefdn::make_unique<remote_key_type []>(num_wks);
        
        for (worker_num_type wk_num = 0; wk_num < num_wks; ++wk_num)
        {
            auto& ep = self.get_ep(wk_num, proc);
            
            // Allocate a new RKEY based on the copied RKEY buffer.
            rkeys[wk_num] = remote_key_type::unpack(uf, ep.get(), ptr_buf+1);
        }
        
        // TODO: Leaking memory.
        const auto minfo_ptr =
            new remote_minfo_type{ mefdn::move(rkeys) };
        
        return remote_ptr_t<T>(static_cast<T*>(raddr), minfo_ptr);
    }
};

} // namespace mecom2
} // namespace menps

