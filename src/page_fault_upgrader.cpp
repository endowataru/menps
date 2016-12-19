
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
            
            auto& space = self.conf_.space;
            auto& indexer = self.conf_.indexer;
            
            if (!indexer.in_range(ptr)) {
                return false;
            }
            
            auto blk_pr = indexer.get_block_accessor_at(ptr);
            
            // Fetch the block first.
            if (! space.fetch(blk_pr))
            {
                // If the block is already readable,
                // then try to make it writable.
                space.touch(blk_pr);
            }
            
            return true;
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

