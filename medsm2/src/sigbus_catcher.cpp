
#include <menps/medsm2/svm/sigbus_catcher.hpp>
#include "signal_util.hpp"

namespace menps {
namespace medsm2 {

namespace /*unnamed*/ {

extern "C"
void sigbus_handler(int, siginfo_t*, void*);

class sigbus_catcher_impl
{
public:
    explicit sigbus_catcher_impl(const sigbus_catcher::config& conf)
        : conf_(conf)
        , replaced_(false)
    {
        self_ = this;
        
        struct sigaction sa;
        
        // We defer SIGSEGV and SIGBUS during handler execution.
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, SIGSEGV);
        sigaddset(&sa.sa_mask, SIGBUS);
        
        // Set the handler function to sa_sigaction
        // The member sa_handler is unset.
        sa.sa_sigaction = &sigbus_handler;
        
        // Signal handler obtains three arguments.
        sa.sa_flags = SA_SIGINFO;
        
        if (conf.alter_stack)
        {
            sa.sa_flags |= SA_ONSTACK;
        }
        
        call_sigaction(SIGBUS, &sa, &old_sa_);
        
        replaced_ = true;
        
        MEFDN_LOG_DEBUG("msg:Replaced SIGBUS handler.");
    }
    
    ~sigbus_catcher_impl()
    {
        restore_old();
    }
    
    static void handle_signal(const int /*signum*/, siginfo_t* const si, void* const uc_void)
    {
        void* const ptr = si->si_addr;
        const auto uc = static_cast<ucontext_t*>(uc_void);
        const auto fault_kind = get_fault_kind(uc);
        
        MEFDN_LOG_VERBOSE(
            "msg:Entering SIGBUS handler.\t"
            "ptr:{:x}\t"
            "sp:{:x}\t"
            "fault_kind:{}"
        ,   reinterpret_cast<mefdn::uintptr_t>(ptr)
        ,   reinterpret_cast<mefdn::uintptr_t>(MEFDN_GET_STACK_POINTER())
        ,   static_cast<int>(fault_kind)
        );
        
        const bool ret = self_->conf_.on_signal(ptr, fault_kind);
        
        MEFDN_LOG_VERBOSE(
            "msg:Exiting SIGBUS handler.\t"
            "ptr:{:x}\t"
            "sp:{:x}\t"
            "fault_kind:{}"
        ,   reinterpret_cast<mefdn::uintptr_t>(ptr)
        ,   reinterpret_cast<mefdn::uintptr_t>(MEFDN_GET_STACK_POINTER())
        ,   static_cast<int>(fault_kind)
        );
        
        if (!ret) {
            // Send another signal.
            raise(SIGUSR2);
            
            #if 0
            self_->restore_old();
            #endif
        }
    }
    
private:
    void restore_old()
    {
        if (replaced_)
        {
            replaced_ = false;
            
            // Set one-shot handler.
            // Removing this may cause an infinite loop.
            old_sa_.sa_flags |= SA_RESETHAND;
            
            call_sigaction(SIGBUS, &old_sa_, nullptr);
        }
    }
    
    const sigbus_catcher::config conf_;
    bool replaced_;
    struct sigaction old_sa_;
    
    // Because a signal handler cannot take user-defined data as void*,
    // there is only a choice to use global variable here.
    static sigbus_catcher_impl* self_;
};

sigbus_catcher_impl* sigbus_catcher_impl::self_ = nullptr;

extern "C"
void sigbus_handler(const int signum, siginfo_t* const si, void* const uc)
{
    sigbus_catcher_impl::handle_signal(signum, si, uc);
}

} // unnamed namespace

class sigbus_catcher::impl
{
public:
    explicit impl(const config& conf)
        : impl_{conf} { }
    
private:
    sigbus_catcher_impl impl_;
};

sigbus_catcher::sigbus_catcher(const config& conf)
    : impl_{new impl{conf}} { }

sigbus_catcher::~sigbus_catcher() = default;

} // namespace medsm2
} // namespace menps

