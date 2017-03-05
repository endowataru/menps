
#include "page_fault_upgrader.hpp"
#include "sigsegv_catcher.hpp"
#include "sigbus_catcher.hpp"

namespace mgdsm {

class page_fault_upgrader::impl
{
public:
    explicit impl(const config& conf)
        : conf_(conf)
        , segv_catch_({ fault_callback{*this}, true })
        , bus_catch_({ fault_callback{*this}, true })
    { }
    
    void enable_on_this_thread()
    {
        is_upgrade_enabled_ = true;
    }
    void disable_on_this_thread()
    {
        is_upgrade_enabled_ = false;
    }
    
private:
    struct fault_callback
    {
        impl& self;
        
        bool operator () (void* const ptr) const
        {
            if (!is_upgrade_enabled_) {
                return false;
            }
            
            self.conf_.sp.do_for_block_at(ptr, upgrade_callback{});
            #if 0
            auto& indexer = self.conf_.indexer;
            
            if (!indexer.in_range(ptr)) {
                return false;
            }
            
            const auto r =
                indexer.do_for_block_at(upgrade_callback{self}, ptr);
            
            
            
            // Note: Histories must be modified after unlocking sharer entries
            //       in order to avoid deadlocking.
            
            if (r.add_read) {
                // Add this block as a flushed page.
                self.conf_.hist.add_new_read(r.ablk_id); 
            }
            if (r.add_write) {
                // Add this block as a reconciled page.
                self.conf_.hist.add_new_write(r.ablk_id);
            }
            #endif
            
            return true;
        }
    };
    
    struct upgrade_error : std::exception { };
    
    struct upgrade_callback
    {
        void operator() (protector_block_accessor& blk_ac)
        {
            // Try to fetch the block first.
            if (blk_ac.fetch())
            {
                // Fetch succeeded. Return to the user code.
            }
            else {
                // If the block is already readable,
                // then try to make it writable.
                if (! blk_ac.touch())
                {
                    // If the block is already touched,
                    // it's OS page must not be protected.
                    MGBASE_ASSERT(false);
                    
                    // The program should abort instead of an exception
                    // because this is in a signal handler.
                    // TODO: Really?
                    abort();
                }
            }
        }
    };
    
        #if 0
    struct upgrade_result
    {
        abs_block_id    ablk_id;
        bool            add_read;
        bool            add_write;
    };
    
    struct upgrade_callback
    {
        impl& self;
        
        upgrade_result operator() (sharer_block::accessor& blk_ac)
        {
            auto& space = self.conf_.space;
            
            const access_history::abs_block_id ablk_id{
                blk_ac.get_segment_id()
            ,   blk_ac.get_page_id()
            ,   blk_ac.get_block_id()
            };
            
            // Fetch the block first.
            if (space.fetch(blk_ac))
            {
                // Call add_new_read() later.
                return { ablk_id, true, false };
            }
            else {
                // If the block is already readable,
                // then try to make it writable.
                if (! space.touch(blk_ac))
                {
                    // If the block is already touched,
                    // it's OS page must not be protected.
                    MGBASE_ASSERT(false);
                    
                    // The program should abort instead of an exception
                    // because this is in a signal handler.
                    // TODO: Really?
                    abort();
                }
                
                // Call add_new_write() later.
                return { ablk_id, false, true };
            }
        }
    };
    
        #endif
    const config conf_;
    
    sigsegv_catcher segv_catch_;
    sigbus_catcher bus_catch_;
    
    static MGBASE_THREAD_LOCAL bool is_upgrade_enabled_;
};

MGBASE_THREAD_LOCAL bool page_fault_upgrader::impl::is_upgrade_enabled_ = false;


page_fault_upgrader::page_fault_upgrader(const config& conf)
    : impl_(new impl(conf))
{ }

page_fault_upgrader::~page_fault_upgrader() /*noexcept*/ = default;

void page_fault_upgrader::enable_on_this_thread() {
    impl_->enable_on_this_thread();
}
void page_fault_upgrader::disable_on_this_thread() {
    impl_->disable_on_this_thread();
}

} // namespace mgdsm

