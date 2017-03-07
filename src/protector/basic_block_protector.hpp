
#pragma once

#include <mgbase/crtp_base.hpp>
#include <unistd.h>
#include <sys/mman.h>
#include <mgbase/memory/next_in_bytes.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/logger.hpp>

namespace mgdsm {

struct block_protector_error { };

template <typename Policy>
class basic_block_protector
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
    typedef typename Policy::sharer_block_accessor_type sharer_block_accessor_type;
    
    static const mgbase::size_t os_page_size = 4096;
    
public:
    void set_invalid()
    {
        auto& self = this->derived();
        auto& sh_blk = self.get_sharer_block_accessor();
        
        this->call_mprotect(sh_blk, PROT_NONE);
    }
    
    void set_readonly()
    {
        auto& self = this->derived();
        auto& sh_blk = self.get_sharer_block_accessor();
        
        this->call_mprotect(sh_blk, PROT_READ);
    }
    
    void set_writable()
    {
        auto& self = this->derived();
        auto& sh_blk = self.get_sharer_block_accessor();
        
        this->call_mprotect(sh_blk, PROT_READ | PROT_WRITE);
    }
    
private:
    void call_mprotect(sharer_block_accessor_type& blk_ac, const int prot)
    {
        auto& self = this->derived();
        
        // Important: This is an index "in a segment".
        const auto index_in_seg = blk_ac.get_index_in_segment();
        const auto size = blk_ac.get_block_size();
        
        const auto seg_id =
            blk_ac.get_page_accessor()
                .get_segment_accessor()
                .get_segment_id();
        
        const auto max_seg_size = self.get_max_seg_size();
        
        const auto index = seg_id * max_seg_size + index_in_seg;
        
        const auto ptr = reinterpret_cast<void*>(index);
        
        MGBASE_ASSERT(reinterpret_cast<mgbase::uintptr_t>(ptr) % os_page_size == 0);
        MGBASE_ASSERT(size % os_page_size == 0);
        
        const int ret = mprotect(ptr, size, prot);
         
        if (ret == 0) {
            MGBASE_LOG_INFO(
                "msg:Called mprotect().\t"
                "ptr:{:x}\t"
                "size_in_bytes:{}\t"
                "prot:{}\t"
            ,   reinterpret_cast<mgbase::uintptr_t>(ptr)
            ,   size
            ,   prot
            );
        }
        else {
            MGBASE_LOG_WARN(
                "msg:mprotect() failed.\t"
                "ptr:{:x}\t"
                "size_in_bytes:{}\n"
                "prot:{}\t"
                "ret:{}\t"
                "errno:{}"
            ,   reinterpret_cast<mgbase::uintptr_t>(ptr)
            ,   size
            ,   prot
            ,   ret
            ,   errno
            );
            
            throw block_protector_error{};
        }
    }
};

} // namespace mgdsm

