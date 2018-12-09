
#include "page_fault_upgrader.hpp"
#include "sigsegv_catcher.hpp"
#include "sigbus_catcher.hpp"
#include <menps/medsm/ult.hpp>

namespace menps {
namespace medsm {

#define MEDSM_USE_THREAD_SPECIFIC

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
        #ifdef MEDSM_USE_THREAD_SPECIFIC
        delete tls_.get();
        tls_.set(new tls_data{0});
        #else
        is_upgrade_enabled_ = true;
        #endif
    }
    void disable_on_this_thread()
    {
        #ifdef MEDSM_USE_THREAD_SPECIFIC
        delete tls_.get();
        tls_.set(nullptr);
        #else
        is_upgrade_enabled_ = false;
        #endif
    }
    
private:
    struct fault_callback
    {
        impl& self;
        
        bool operator () (void* const ptr) const
        {
            #ifdef MEDSM_USE_THREAD_SPECIFIC
            auto p = self.tls_.get();
            if (p == nullptr) {
                return false;
            }
            
            if (++p->rec_count > 1) {
                return false;
            }
            #else
            if (!is_upgrade_enabled_) {
                return false;
            }
            
            if (++rec_count_ > 1) {
                return false;
            }
            #endif
            
            if (ptr == nullptr) {
                return false;
            }
            
            self.conf_.sp.do_for_block_at(ptr, upgrade_callback{});
            
            #ifdef MEDSM_USE_THREAD_SPECIFIC
            --p->rec_count;
            #else
            --rec_count_;
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
                    MEFDN_ASSERT(false);
                    
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
 
    #ifdef MEDSM_USE_THREAD_SPECIFIC
    struct tls_data {
        mefdn::size_t  rec_count;
    };
    struct tls_policy {
        typedef tls_data    value_type;
    };
    
    medsm::ult::thread_specific<tls_policy> tls_;
    
    #else
    static MEFDN_THREAD_LOCAL bool is_upgrade_enabled_;
    static MEFDN_THREAD_LOCAL mefdn::size_t rec_count_;
    #endif
};

#ifndef MEDSM_USE_THREAD_SPECIFIC
MEFDN_THREAD_LOCAL bool page_fault_upgrader::impl::is_upgrade_enabled_ = false;
MEFDN_THREAD_LOCAL mefdn::size_t page_fault_upgrader::impl::rec_count_ = 0;
#endif


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

} // namespace medsm
} // namespace menps

