
#pragma once

#include <menps/mefdn/crtp_base.hpp>
#include <menps/mefdn/memory/next_in_bytes.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/logger.hpp>

#include <unistd.h>
#include <sys/mman.h>

namespace menps {
namespace medsm {

struct block_protector_error { };

template <typename Policy>
class basic_block_protector
{
    MEFDN_POLICY_BASED_CRTP(Policy)
    
    typedef typename Policy::sharer_block_accessor_type sharer_block_accessor_type;
    
    static const mefdn::size_t os_page_size = 4096;
    
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
        
        const auto seg_app_ptr = self.get_segment_app_ptr();
        
        const auto ptr = mefdn::next_in_bytes(seg_app_ptr, index_in_seg);
        
        MEFDN_ASSERT(reinterpret_cast<mefdn::uintptr_t>(ptr) % os_page_size == 0);
        MEFDN_ASSERT(size % os_page_size == 0);
        
        const int ret = mprotect(ptr, size, prot);
         
        if (ret == 0) {
            MEFDN_LOG_INFO(
                "msg:Called mprotect().\t"
                "ptr:{:x}\t"
                "size_in_bytes:{}\t"
                "prot:{}\t"
            ,   reinterpret_cast<mefdn::uintptr_t>(ptr)
            ,   size
            ,   prot
            );
        }
        else {
            MEFDN_LOG_WARN(
                "msg:mprotect() failed.\t"
                "ptr:{:x}\t"
                "size_in_bytes:{}\n"
                "prot:{}\t"
                "ret:{}\t"
                "errno:{}"
            ,   reinterpret_cast<mefdn::uintptr_t>(ptr)
            ,   size
            ,   prot
            ,   ret
            ,   errno
            );
            
            throw block_protector_error{};
        }
    }
};

} // namespace medsm
} // namespace menps

