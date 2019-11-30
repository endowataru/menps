
#include <menps/medsm2/svm/sigsegv_catcher.hpp>
#include "signal_util.hpp"

namespace menps {
namespace medsm2 {

namespace /*unnamed*/ {

extern "C"
void sigsegv_handler(int, siginfo_t*, void*);

class sigsegv_catcher_impl
{
public:
    explicit sigsegv_catcher_impl(const sigsegv_catcher::config& conf)
        : conf_(conf)
        , replaced_(false)
    {
        self_ = this;
        
        struct sigaction sa;
        
        // We defer SIGSEGV and SIGBUS during handler execution.
        sigemptyset(&sa.sa_mask);
        //sigaddset(&sa.sa_mask, SIGSEGV);
        sigaddset(&sa.sa_mask, SIGBUS);
        
        // Set the handler function to sa_sigaction
        // The member sa_handler is unset.
        sa.sa_sigaction = &sigsegv_handler;
        
        // Signal handler obtains three arguments.
        sa.sa_flags = SA_SIGINFO | SA_NODEFER;
        
        if (conf.alter_stack)
        {
            sa.sa_flags |= SA_ONSTACK;
        }
        
        call_sigaction(SIGSEGV, &sa, &old_sa_);
        
        replaced_ = true;
        
        MEFDN_LOG_DEBUG("msg:Replaced SIGSEGV handler.");
    }
    
    ~sigsegv_catcher_impl()
    {
        restore_old();
        
        MEFDN_LOG_DEBUG("msg:Restored original SIGSEGV handler.");
    }
    
    static void handle_signal(const int /*signum*/, siginfo_t* const si, void* const uc_void)
    {
        void* const ptr = si->si_addr;
        const auto uc = static_cast<ucontext_t*>(uc_void);
        const auto fault_kind = get_fault_kind(uc);
        
        MEFDN_LOG_VERBOSE(
            "msg:Entering SIGSEGV handler.\t"
            "ptr:{:x}\t"
            "sp:{:x}\t"
            "fault_kind:{}"
        ,   reinterpret_cast<mefdn::uintptr_t>(ptr)
        ,   reinterpret_cast<mefdn::uintptr_t>(MEFDN_GET_STACK_POINTER())
        ,   static_cast<int>(fault_kind)
        );
        
        const bool ret = self_->conf_.on_signal(ptr, fault_kind);
        
        MEFDN_LOG_VERBOSE(
            "msg:Exiting SIGSEGV handler.\t"
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
            
            call_sigaction(SIGSEGV, &old_sa_, nullptr);
        }
    }
    
    const sigsegv_catcher::config conf_;
    bool replaced_;
    struct sigaction old_sa_;
    
    // Because a signal handler cannot take user-defined data as void*,
    // there is only a choice to use global variable here.
    static sigsegv_catcher_impl* self_;
};

sigsegv_catcher_impl* sigsegv_catcher_impl::self_ = nullptr;

extern "C"
void sigsegv_handler(const int signum, siginfo_t* const si, void* const uc)
{
    sigsegv_catcher_impl::handle_signal(signum, si, uc);
}

} // unnamed namespace

class sigsegv_catcher::impl
{
public:
    explicit impl(const config& conf)
        : impl_{conf} { }
    
private:
    sigsegv_catcher_impl impl_;
};

sigsegv_catcher::sigsegv_catcher(const config& conf)
    : impl_{new impl{conf}} { }

sigsegv_catcher::~sigsegv_catcher() = default;

} // namespace medsm2
} // namespace menps

