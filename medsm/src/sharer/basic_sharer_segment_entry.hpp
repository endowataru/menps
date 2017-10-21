
#pragma once

#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/arithmetic.hpp>

namespace menps {
namespace medsm {

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
        : manager_(mefdn::move(conf.manager))
        , sys_ptr_(conf.sys_ptr)
        
        // Copy from the manager.
        , pg_size_(manager_->get_page_size())
        , num_pgs_(manager_->get_num_pages())
        
        // Create sharer page entries.
        , pgs_(mefdn::make_unique<page_type []>(num_pgs_))
    {
        // Load the default block size.
        const auto blk_size = manager_->get_block_size();
        const auto num_blks = mefdn::roundup_divide(pg_size_, blk_size);
        
        for (page_id_type pg_id = 0; pg_id < num_pgs_; ++pg_id) {
            auto& pg = pgs_[pg_id];
            pg.set_block_size(blk_size);
            pg.set_num_blocks(num_blks);
        }
    }
    
    page_type& get_page(const page_id_type pg_id) const noexcept
    {
        MEFDN_ASSERT(pg_id < this->get_num_pages());
        
        return pgs_[pg_id];
    }
    
    mefdn::size_t get_page_size() const noexcept {
        return pg_size_;
    }
    
    manager_type& get_manager() const noexcept {
        return *manager_;
    }
    
    void* get_sys_ptr() const noexcept {
        return sys_ptr_;
    }
    
private:
    mefdn::size_t get_num_pages() const noexcept {
        return num_pgs_;
    }
    
    manager_ptr_type                    manager_;
    void*                               sys_ptr_;
    
    mefdn::size_t                      pg_size_;
    mefdn::size_t                      num_pgs_;
    
    mefdn::unique_ptr<page_type []>    pgs_;
};

} // namespace medsm
} // namespace menps

