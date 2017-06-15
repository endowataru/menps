
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
            
            if (++rec_count_ > 1) {
                return false;
            }
            
            if (ptr == MGBASE_NULLPTR) {
                return false;
            }
            
            self.conf_.sp.do_for_block_at(ptr, upgrade_callback{});
            
            --rec_count_;
            
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
   
    const config conf_;
    
    sigsegv_catcher segv_catch_;
    sigbus_catcher bus_catch_;
    
    static MGBASE_THREAD_LOCAL bool is_upgrade_enabled_;
    static MGBASE_THREAD_LOCAL mgbase::size_t rec_count_;
};

MGBASE_THREAD_LOCAL bool page_fault_upgrader::impl::is_upgrade_enabled_ = false;
MGBASE_THREAD_LOCAL mgbase::size_t page_fault_upgrader::impl::rec_count_ = 0;


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

