
#pragma once

#include <cmpth/wss/basic_worker.hpp>
#include <cmpth/wss/basic_worker_task.hpp>
#include <cmpth/wss/basic_worker_tls.hpp>
#include <cmpth/sct/sct_running_task.hpp>

namespace cmpth {

// level 3: worker

template <typename P>
struct sct_worker_policy
{
private:
    using lv2_itf_type = typename P::lv2_itf_type;
    
public:
    using derived_type = basic_worker<sct_worker_policy>;
    
    // lv3
    
    using worker_num_type = typename P::worker_num_type;
    static const worker_num_type invalid_worker_num = P::invalid_worker_num;
    
    using worker_task_type = basic_worker_task<sct_worker_policy>;
    using running_task_type = sct_running_task<sct_worker_policy>;
    using worker_tls_type = basic_worker_tls<sct_worker_policy>;
    
    // lv2
    
    using worker_deque_type     = typename lv2_itf_type::worker_deque;
    
    // lv1
    
    using task_desc_type        = typename lv2_itf_type::task_desc;
    using call_stack_type       = typename lv2_itf_type::call_stack;
    using continuation_type     = typename lv2_itf_type::continuation;
    using task_ref_type         = typename lv2_itf_type::task_ref;
    using unique_task_ptr_type  = typename lv2_itf_type::unique_task_ptr;
    
    using context_policy_type   = typename lv2_itf_type::context_policy;
    using context_type          = typename lv2_itf_type::context;
    using transfer_type         = typename lv2_itf_type::transfer;
    using cond_transfer_type    = typename lv2_itf_type::cond_transfer;
    
    using base_ult_itf_type     = typename lv2_itf_type::base_ult_itf;
    
    using assert_policy_type    = typename lv2_itf_type::assert_policy;
    using log_policy_type       = typename lv2_itf_type::log_policy;
};

template <typename P>
using sct_worker = basic_worker<sct_worker_policy<P>>;

template <typename P>
struct lv3_sct_itf
    : P::lv2_itf_type
{
public:
    using worker = sct_worker<P>;
    
    using worker_num_type = typename P::worker_num_type;
    
    static worker_num_type get_worker_num() noexcept {
        const auto& wk = worker::get_cur_worker();
        return wk.get_worker_num();
    }
    static worker_num_type get_num_workers() noexcept {
        // TODO: There is a circular dependency to the scheduler.
        return P::get_num_workers();
    }
    
    using typename P::lv2_itf_type::task_desc;
    
    struct this_thread {
        static void yield() {
            auto& wk = worker::get_cur_worker();
            wk.yield();
        }
        
        static task_desc* native_handle() noexcept {
            auto& wk = worker::get_cur_worker();
            const auto tk = wk.get_cur_task_ref();
            return tk.get_task_desc();
        }
    };
};

} // namespace cmpth

