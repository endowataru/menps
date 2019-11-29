
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/medev2/ucx/uct/uct.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_uct_request_object
    : private uct_completion_t
{
    using comp_state_type = typename P::comp_state_type;
    using atomic_comp_state_type = typename P::atomic_comp_state_type;
    
    using uct_itf_type = typename P::uct_itf_type;
    using endpoint_type = typename uct_itf_type::endpoint_type;

    using ult_itf_type = typename P::ult_itf_type;
    using worker_type = typename ult_itf_type::worker;
    using suspended_thread_type = typename ult_itf_type::suspended_thread;
    
    using uct_rma_type = typename P::uct_rma_type;
    using rkey_info_type = typename P::rkey_info_type;
    using worker_unique_lock_type = typename P::worker_unique_lock_type;

    using proc_id_type = typename P::proc_id_type;
    template <typename T>
    using remote_ptr_t = typename P::template remote_ptr<T>;

public:
    /*implicit*/ basic_uct_request_object()
        : uct_completion_t{&on_complete, 1}
        , state_{comp_state_type::created}
    { }
    
    basic_uct_request_object(const basic_uct_request_object&) = delete;
    basic_uct_request_object& operator = (const basic_uct_request_object&) = delete;
    
    void start(
        uct_rma_type&                   rma
    ,   const proc_id_type              proc
    ,   const remote_ptr_t<const void>& rptr
    ) {
        this->rkey_info_ = rma.lock_rma(proc, rptr);
    }

    void finish(uct_rma_type& rma)
    {
        MEFDN_ASSERT(this->state_.load() == comp_state_type::created);
        this->state_.store(comp_state_type::finished, ult_itf_type::memory_order_relaxed);
        rma.unlock_rma(this->rkey_info_);
    }
    
    void wait(uct_rma_type& rma)
    {
        #ifdef MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS
        while (true) {
            auto st = this->state_.load(ult_itf_type::memory_order_acquire);
            if (st == comp_state_type::finished) {
                break;
            }
            
            ult_itf_type::this_thread::yield();
            
            const auto lk = this->lock_worker_lock();
            while (this->rkey_info_.iface->progress() != 0) {
                // Poll until there are completions
            }
        }

        #else
        const auto st = this->state_.load(ult_itf_type::memory_order_acquire);
        if (st != comp_state_type::finished) {
            MEFDN_ASSERT(st == comp_state_type::created);
            this->sth_.template wait_with<on_wait>(this);
        }
        #endif
        
        rma.unlock_rma(this->rkey_info_);
    }

private:
    struct on_wait {
        bool operator() (
            worker_type&                    /*wk*/
        ,   basic_uct_request_object* const self
        ) const noexcept
        {
            auto expected = comp_state_type::created;
            
            return self->state_.compare_exchange_strong(
                expected
            ,   comp_state_type::waiting
            ,   ult_itf_type::memory_order_acq_rel
            ,   ult_itf_type::memory_order_relaxed
            );
        }
    };
    
public:
    uct_completion_t* get_completion_ptr() noexcept {
        return this;
    }
    endpoint_type& get_ep() noexcept {
        MEFDN_ASSERT(this->rkey_info_.ep != nullptr);
        return *this->rkey_info_.ep;
    }
    const uct_rkey_bundle_t& get_rkey_bundle() noexcept {
        MEFDN_ASSERT(this->rkey_info_.rkey != nullptr);
        return this->rkey_info_.rkey->get();
    }
    worker_unique_lock_type lock_worker_lock() {
        #ifdef MECOM2_UCT_RMA_ENABLE_WORKER_LOCK
        MEFDN_ASSERT(this->rkey_info_.lock != nullptr);
        return worker_unique_lock_type{*this->rkey_info_.lock};
        #else
        return worker_unique_lock_type{};
        #endif
    }
    uct_iov& get_iov() noexcept { return this->iov_; }
    
private:
    static void on_complete(
        uct_completion_t* const comp
    ,   const ucs_status_t      status MEFDN_MAYBE_UNUSED
    ) {
        // Static downcast.
        auto& self = *static_cast<basic_uct_request_object*>(comp);
        
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
        self.state_.store(comp_state_type::finished, ult_itf_type::memory_order_release);
        
        #else
        auto st = self.state_.load(ult_itf_type::memory_order_acquire);
        
        bool waiting = st == comp_state_type::waiting;
        if (!waiting) {
            MEFDN_ASSERT(st == comp_state_type::created);
            waiting =
                ! self.state_.compare_exchange_strong(
                    st
                ,   comp_state_type::finished
                ,   ult_itf_type::memory_order_acq_rel
                ,   ult_itf_type::memory_order_relaxed
                );
        }
        
        if (waiting) {
            // Prefer returning to the progress thread immediately.
            self.sth_.notify();
        }
        #endif
    }
    
    atomic_comp_state_type  state_;
    rkey_info_type          rkey_info_;
    suspended_thread_type   sth_;
    uct_iov                 iov_;
};

} // namespace mecom2
} // namespace menps

