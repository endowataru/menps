
#pragma once

#include <menps/mecom2/common.hpp>
#ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
    #include <menps/mecom2/com/com_signal_state.hpp>
#endif
#include <menps/medev2/ucx/uct/uct.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_uct_completion
    : private uct_completion_t
{
    using comp_state_type = typename P::comp_state_type;
    using atomic_comp_state_type = typename P::atomic_comp_state_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using spinlock_type = typename ult_itf_type::spinlock;
    using uncond_variable_type = typename ult_itf_type::uncond_variable;
    
    using rkey_info_type = typename P::rkey_info_type;
    
public:
    basic_uct_completion()
        : uct_completion_t{ &on_complete, 1 }
        , state_(comp_state_type::created)
    { }
    
private:
    struct on_wait {
        basic_uct_completion& self;
        
        bool operator() () const noexcept
        {
            auto expected = comp_state_type::created;
            
            return self.state_.compare_exchange_strong(
                expected
            ,   comp_state_type::waiting
            ,   mefdn::memory_order_acq_rel
            ,   mefdn::memory_order_relaxed
            );
        }
    };
    
public:
    void wait(const rkey_info_type& info MEFDN_MAYBE_UNUSED)
    {
        #ifdef MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS
            while (true)
            {
                auto st = this->state_.load(mefdn::memory_order_acquire);
                
                if (st == comp_state_type::finished) {
                    break;
                }
                
                #ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
                if (! com_signal_state::is_in_signal()) {
                    ult_itf_type::this_thread::yield();
                }
                #else
                ult_itf_type::this_thread::yield();
                #endif
                
                {
                    #ifdef MECOM2_UCT_RMA_ENABLE_WORKER_LOCK
                    mefdn::lock_guard<spinlock_type> lk(info.lock);
                    #endif
                    
                    while (info.iface.progress() != 0) {
                        // Poll until there are completions
                    }
                }
            }
        #else
            const auto st = this->state_.load(mefdn::memory_order_acquire);
            if (st != comp_state_type::finished) {
                MEFDN_ASSERT(st == comp_state_type::created);
                this->uv_.wait_with(on_wait{ *this });
            }
        #endif
    }
    
    uct_completion_t* get_ptr() noexcept {
        return this;
    }
    
private:
    static void on_complete(
        uct_completion_t* const comp
    ,   const ucs_status_t      status MEFDN_MAYBE_UNUSED
    ) {
        // Static downcast.
        auto& self = *static_cast<basic_uct_completion*>(comp);
        
        MEFDN_LOG_VERBOSE(
            "msg:Completion function was called.\t"
            "self:0x{:x}\t"
            "count:{}\t"
            "status:{}\t"
        ,   reinterpret_cast<mefdn::intptr_t>(&self)
        ,   self.count
        ,   status
        );
        
        #ifdef MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS
            self.state_.store(comp_state_type::finished, mefdn::memory_order_release);
        #else
            auto st = self.state_.load(mefdn::memory_order_acquire);
            
            bool waiting = (st == comp_state_type::waiting);
            if (!waiting) {
                MEFDN_ASSERT(st == comp_state_type::created);
                
                waiting =
                    ! self.state_.compare_exchange_strong(
                        st
                    ,   comp_state_type::finished
                    ,   mefdn::memory_order_acq_rel
                    ,   mefdn::memory_order_relaxed
                    );
            }
            
            if (waiting) {
                // Prefer returning to the progress thread immediately.
                self.uv_.notify_signal();
            }
        #endif
    }
    
    atomic_comp_state_type  state_;
    uncond_variable_type    uv_;
};

} // namespace mecom2
} // namespace menps

