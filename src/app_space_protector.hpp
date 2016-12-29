
#pragma once

#include "sharer/sharer_block_accessor.hpp"
#include <unistd.h>
#include <sys/mman.h>
#include <mgbase/logger.hpp>

namespace mgdsm {

struct app_space_protector_error : std::exception { };

class app_space_protector
{
    static const mgbase::size_t os_page_size = 4096;
    
public:
    struct config
    {
        void*           app_ptr;
        mgbase::size_t  seg_size;
    };
    
    explicit app_space_protector(const config& conf)
        : conf_(conf)
    { }
    
    app_space_protector(const app_space_protector&) = delete;
    app_space_protector& operator = (const app_space_protector&) = delete;
    
    void set_invalid(sharer_block::accessor& blk_pr)
    {
        this->call_mprotect(blk_pr, PROT_NONE);
    }
    
    void set_readonly(sharer_block::accessor& blk_pr)
    {
        this->call_mprotect(blk_pr, PROT_READ);
    }
    
    void set_writable(sharer_block::accessor& blk_pr)
    {
        this->call_mprotect(blk_pr, PROT_READ | PROT_WRITE);
    }
    
private:
    void call_mprotect(sharer_block::accessor& blk_pr, const int prot)
    {
        // Important: This is an index "in a segment".
        const auto index_in_seg = blk_pr.get_index_in_segment();
        const auto size = blk_pr.get_block_size();
        
        const auto seg_id =
            blk_pr.get_page_accessor()
                .get_segment_accessor()
                .get_segment_id();
        
        const auto index = seg_id * this->conf_.seg_size + index_in_seg;
        
        const auto ptr = mgbase::next_in_bytes(this->conf_.app_ptr, index);
        
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
            
            throw app_space_protector_error{};
        }
    }
    
    const config conf_;
};

} // namespace mgdsm

