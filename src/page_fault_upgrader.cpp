
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
            
            auto& indexer = self.conf_.indexer;
            
            if (!indexer.in_range(ptr)) {
                return false;
            }
            
            indexer.do_for_block_at(upgrade_callback{self}, ptr);
            
            return true;
        }
    };
    
    struct upgrade_error : std::exception { };
    
    struct upgrade_callback
    {
        impl& self;
        
        void operator() (sharer_block::accessor& blk_ac)
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
                // Add this block as a flushed page.
                self.conf_.hist.add_new_read(ablk_id);
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
                
                // Add this block as a reconciled page.
                self.conf_.hist.add_new_write(ablk_id);
            }
        }
    };
    
    const config conf_;
    
    sigsegv_catcher segv_catch_;
    sigbus_catcher bus_catch_;
    
    MGBASE_THREAD_LOCAL static bool is_upgrade_enabled_;
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

