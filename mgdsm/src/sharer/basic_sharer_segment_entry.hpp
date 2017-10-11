
#pragma once

#include <mgbase/unique_ptr.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/utility/move.hpp>
#include <mgbase/arithmetic.hpp>

namespace mgdsm {

template <typename Traits>
class basic_sharer_segment_entry
{
    typedef typename Traits::derived_type       derived_type;
    
    typedef typename Traits::page_type          page_type;
    typedef typename Traits::page_id_type       page_id_type;
    
    typedef typename Traits::manager_type       manager_type;
    typedef typename Traits::manager_ptr_type   manager_ptr_type;
    
public:
    template <typename Conf>
    explicit basic_sharer_segment_entry(Conf&& conf)
        // Copy from the configuration.
        : manager_(mgbase::move(conf.manager))
        , sys_ptr_(conf.sys_ptr)
        
        // Copy from the manager.
        , pg_size_(manager_->get_page_size())
        , num_pgs_(manager_->get_num_pages())
        
        // Create sharer page entries.
        , pgs_(mgbase::make_unique<page_type []>(num_pgs_))
    {
        // Load the default block size.
        const auto blk_size = manager_->get_block_size();
        const auto num_blks = mgbase::roundup_divide(pg_size_, blk_size);
        
        for (page_id_type pg_id = 0; pg_id < num_pgs_; ++pg_id) {
            auto& pg = pgs_[pg_id];
            pg.set_block_size(blk_size);
            pg.set_num_blocks(num_blks);
        }
    }
    
    page_type& get_page(const page_id_type pg_id) const MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(pg_id < this->get_num_pages());
        
        return pgs_[pg_id];
    }
    
    mgbase::size_t get_page_size() const MGBASE_NOEXCEPT {
        return pg_size_;
    }
    
    manager_type& get_manager() const MGBASE_NOEXCEPT {
        return *manager_;
    }
    
    void* get_sys_ptr() const MGBASE_NOEXCEPT {
        return sys_ptr_;
    }
    
private:
    mgbase::size_t get_num_pages() const MGBASE_NOEXCEPT {
        return num_pgs_;
    }
    
    manager_ptr_type                    manager_;
    void*                               sys_ptr_;
    
    mgbase::size_t                      pg_size_;
    mgbase::size_t                      num_pgs_;
    
    mgbase::unique_ptr<page_type []>    pgs_;
};

} // namespace mgdsm

