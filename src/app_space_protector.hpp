
#pragma once

#include "sharer/sharer_block_accessor.hpp"
#include <unistd.h>
#include <sys/mman.h>

namespace mgdsm {

class app_space_protector
{
    static const mgbase::size_t os_page_size = 4096;
    
public:
    explicit app_space_protector(void* const app_ptr)
        : app_ptr_(app_ptr)
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
        const auto index = blk_pr.get_index_in_segment();
        const auto size = blk_pr.get_block_size();
        
        const auto ptr = mgbase::next_in_bytes(this->app_ptr_, index);
        
        MGBASE_ASSERT(reinterpret_cast<mgbase::uintptr_t>(ptr) % os_page_size == 0);
        MGBASE_ASSERT(size % os_page_size == 0);
        
        mprotect(ptr, size, prot);
    }
    
    void* const app_ptr_;
};

} // namespace mgdsm

