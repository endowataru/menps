
#pragma once

#include <mgbase/assert.hpp>
#include <mgbase/unique_ptr.hpp>
#include <vector>
#include <mgbase/atomic.hpp>
#include <mgbase/threading/unique_lock.hpp>
#include <mgbase/threading/spinlock.hpp>

namespace mgdsm {

template <typename Traits>
class basic_sharer_page_entry
{
    typedef typename Traits::block_type         block_type;
    typedef typename Traits::block_entry_type   block_entry_type;
    typedef typename Traits::block_id_type      block_id_type;
    typedef typename Traits::block_count_type   block_count_type;
    
    typedef typename Traits::prptr_type         prptr_type;
    
    typedef mgbase::spinlock                        transfer_lock_type;
    typedef mgbase::unique_lock<transfer_lock_type> unique_transfer_lock_type;
    
public:
    basic_sharer_page_entry()
        : blk_size_(0)
        , num_blks_(0)
        , blks_{}
        , owner_()
        , num_read_blks_(0)
        , num_write_blks_(0)
        , is_diff_needed_{false}
        , is_flush_needed_{false}
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
    
    unique_transfer_lock_type get_transfer_lock() {
        return unique_transfer_lock_type(this->transfer_lock_);
    }
    
    bool is_diff_needed(unique_transfer_lock_type& lk) MGBASE_NOEXCEPT {
        check_transfer_locked(lk);
        //return this->is_diff_needed_.load(mgbase::memory_order_acquire);
        return true; // FIXME: broken?
    }
    bool is_flush_needed(unique_transfer_lock_type& lk) MGBASE_NOEXCEPT {
        check_transfer_locked(lk);
        return this->is_flush_needed_.load(mgbase::memory_order_acquire);
        //return true; // FIXME: broken?
    }
    
    void enable_diff(unique_transfer_lock_type& lk) MGBASE_NOEXCEPT
    {
        check_transfer_locked(lk);
        
        // Ignore redundant enable_diff()
        // even if is_flush_needed_ is already true.
        
        this->is_diff_needed_.store(true, mgbase::memory_order_release);
    }
    void enable_flush(unique_transfer_lock_type& lk) MGBASE_NOEXCEPT
    {
        check_transfer_locked(lk);
        
        // Ignore redundant enable_flush()
        // even if is_flush_needed_ is already true.
        
        this->is_flush_needed_.store(true, mgbase::memory_order_release);
    }
    
    template <typename Data>
    void update_owner_for_read(unique_transfer_lock_type& lk, const Data& data)
    {
        check_transfer_locked(lk);
        
        this->owner_ = Traits::use_remote_ptr(data.owner_plptr);
        this->is_flush_needed_.store(data.needs_flush, mgbase::memory_order_release);
        
        // is_diff_needed_ is not updated
        // because the reader doesn't need that flag.
    }
    template <typename Data>
    void update_owner_for_write(unique_transfer_lock_type& lk, const Data& data)
    {
        check_transfer_locked(lk);
        
        this->owner_ = Traits::use_remote_ptr(data.owner_plptr);
        this->is_flush_needed_.store(data.needs_flush, mgbase::memory_order_release);
        
        const bool enable_diff_arrived =
            this->is_diff_needed_.load(mgbase::memory_order_relaxed);
        
        const bool is_diff_needed_new =
            data.needs_diff || enable_diff_arrived;
        
        this->is_diff_needed_.store(is_diff_needed_new, mgbase::memory_order_relaxed);
    }
    template <typename Data>
    void update_for_release_write(unique_transfer_lock_type& lk, const Data& data)
    {
        check_transfer_locked(lk);
        
        this->is_flush_needed_.store(data.needs_flush, mgbase::memory_order_release);
    }
    
    const prptr_type& get_owner_prptr() const MGBASE_NOEXCEPT {
        return this->owner_;
    }
    
private:
    void check_transfer_locked(unique_transfer_lock_type& lk)
    {
        MGBASE_ASSERT(lk.mutex() == &this->transfer_lock_);
        MGBASE_ASSERT(lk.owns_lock());
    }
    
    mgbase::size_t                      blk_size_;
    mgbase::size_t                      num_blks_;
    mgbase::unique_ptr<block_type []>   blks_;
    
    prptr_type              owner_;
    
    block_count_type        num_read_blks_;
    block_count_type        num_write_blks_;
    
    mgbase::spinlock                    transfer_lock_;
    mgbase::atomic<bool>                is_diff_needed_;
    mgbase::atomic<bool>                is_flush_needed_;
};

} // namespace mgdsm

