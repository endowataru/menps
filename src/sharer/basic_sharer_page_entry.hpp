
#pragma once

#include <mgbase/assert.hpp>
#include <mgbase/unique_ptr.hpp>
#include <vector>

namespace mgdsm {

template <typename Traits>
class basic_sharer_page_entry
{
    typedef typename Traits::derived_type       derived_type;
    
    typedef typename Traits::block_type         block_type;
    typedef typename Traits::block_entry_type   block_entry_type;
    typedef typename Traits::block_id_type      block_id_type;
    typedef typename Traits::block_count_type   block_count_type;
    
    typedef typename Traits::prptr_type         prptr_type;
    
public:
    basic_sharer_page_entry()
        : blk_size_(0)
        , num_blks_(0)
        , blks_{}
        , owner_{}
        , num_read_blks_(0)
        , num_write_blks_(0)
        , is_diff_needed_(false)
        , is_flush_needed_(false)
    { }
    
    void set_block_size(const mgbase::size_t blk_size) {
        MGBASE_ASSERT(blk_size > 0);
        this->blk_size_ = blk_size;
    }
    void set_num_blocks(const mgbase::size_t num_blks) {
        MGBASE_ASSERT(num_blks > 0);
        this->num_blks_ = num_blks;
    }
    
    block_type& get_block(const block_id_type blk_id)
    {
        MGBASE_ASSERT(blk_id < this->get_num_blocks());
        
        if (! blks_)
        {
            // Initialize the blocks lazily.
            // TODO: How this affects the performance?A
            
            blks_ = mgbase::make_unique<block_type []>(this->get_num_blocks());
        }
        
        return blks_[blk_id];
    }
    
    mgbase::size_t get_block_size() const MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(this->blk_size_ > 0);
        return this->blk_size_;
    }
    mgbase::size_t get_num_blocks() const MGBASE_NOEXCEPT {
        return this->num_blks_;
    }
    
    bool add_read_block() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(num_read_blks_ < this->get_num_blocks());
        
        return ++num_read_blks_ == 1;
    }
    bool remove_read_block() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(num_read_blks_ > 0);
        
        return --num_read_blks_ == 0;
    }
    bool add_write_block() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(num_write_blks_ < this->get_num_blocks());
        
        return ++num_write_blks_ == 1;  
    }
    bool remove_write_block() MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(num_write_blks_ > 0);
        
        return --num_write_blks_ == 0;
    }
    
    bool is_readable_from_owner() const MGBASE_NOEXCEPT {
        return num_read_blks_ > 0;
    }
    bool is_writable_from_owner() const MGBASE_NOEXCEPT {
        return num_read_blks_ > 0;
    }
    
    bool is_diff_needed() const MGBASE_NOEXCEPT {
        //return this->is_diff_needed_;
        return true; // FIXME: broken?
    }
    bool is_flush_needed() const MGBASE_NOEXCEPT {
        //return this->is_flush_needed_;
        return true; // FIXME: broken?
    }
    
    void enable_diff() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(!this->is_diff_needed_);
        this->is_diff_needed_ = true;
    }
    void enable_flush() MGBASE_NOEXCEPT {
        //MGBASE_ASSERT(!this->is_flush_needed_); // FIXME
        this->is_flush_needed_ = true;
    }
    
    template <typename Data>
    void update_owner_for_read(const Data& data)
    {
        this->owner_            = Traits::use_remote_ptr(data.owner_plptr);
        this->is_flush_needed_  = data.needs_flush;
    }
    template <typename Data>
    void update_owner_for_write(const Data& data)
    {
        this->owner_            = Traits::use_remote_ptr(data.owner_plptr);
        this->is_flush_needed_  = data.needs_flush;
        this->is_diff_needed_   = data.needs_diff;
    }
    
    const prptr_type& get_owner_prptr() const MGBASE_NOEXCEPT {
        return this->owner_;
    }
    
private:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    
    mgbase::size_t                      blk_size_;
    mgbase::size_t                      num_blks_;
    mgbase::unique_ptr<block_type []>   blks_;
    
    prptr_type              owner_;
    
    block_count_type        num_read_blks_;
    block_count_type        num_write_blks_;
    
    bool                    is_diff_needed_;
    bool                    is_flush_needed_;
};

} // namespace mgdsm

