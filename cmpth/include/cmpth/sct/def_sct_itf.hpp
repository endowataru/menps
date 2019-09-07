
#pragma once

#include <cmpth/sct/lv1_sct_itf.hpp>
#include <cmpth/sct/lv3_sct_itf.hpp>
#include <cmpth/sct/lv5_sct_itf.hpp>
#include <cmpth/sct/lv6_sct_itf.hpp>
#include <cmpth/sct/lv7_sct_itf.hpp>

#include <cmpth/sct/def_sct_spinlock.hpp>
#include <cmpth/arch/x86_64_context_policy.hpp>
#include <cmpth/pool/basic_ext_return_pool.hpp>
#include <cmpth/sync/basic_spinlock.hpp>
#include <cmpth/sct/sct_worker_deque.hpp>
#include <unordered_map>

#include <cmpth/ult_tag.hpp>

namespace cmpth {

// level 1: task_desc

struct def_lv3_sct_policy;

struct def_lv1_sct_policy
    : def_sct_common_policy
{
    using context_policy_type = x86_64_context_policy<def_sct_common_policy>;
    
    using task_mutex_type = def_sct_spinlock;   // used only for sct_task_desc
    using spinlock_type = def_sct_spinlock;     // used for other purposes
    
    using tls_key_type = fdn::size_t;
    using tls_map_impl_type = std::unordered_map<tls_key_type, void*>;
    
    using worker_type = sct_worker<def_lv3_sct_policy>; // TODO: circular dependency
};

using def_lv1_sct_itf = lv1_sct_itf<def_lv1_sct_policy>;

// level 2: worker deque

struct def_sct_worker_deque_policy
    : def_sct_common_policy
{
    using continuation_type = def_lv1_sct_itf::continuation;
    using spinlock_type = def_sct_spinlock;
};

struct def_lv2_sct_itf
    : def_lv1_sct_itf
{
    using worker_deque = sct_worker_deque<def_sct_worker_deque_policy>;
};

// level 3: worker

struct def_lv3_sct_policy {
    using lv2_itf_type = def_lv2_sct_itf;
    
    using worker_num_type = fdn::int32_t;
    static const worker_num_type invalid_worker_num = -1;
    
    inline static worker_num_type get_num_workers() noexcept;
};

using def_lv3_sct_itf = lv3_sct_itf<def_lv3_sct_policy>;

// level 4: pool

struct def_lv4_sct_itf
    : def_lv3_sct_itf
{
    template <typename P2>
    using pool_t = basic_ext_return_pool<def_lv3_sct_itf, P2>;
};

// level 5: thread, scheduler

struct def_lv5_sct_policy {
    using lv4_itf_type = def_lv4_sct_itf;
    
    static fdn::size_t get_default_stack_size() noexcept {
        return constants::default_stack_size;
    }
    template <typename Pool>
    static fdn::size_t get_task_pool_threshold(Pool& /*pool*/) noexcept {
        return constants::default_task_pool_threshold;
    }
};

using def_lv5_sct_itf = lv5_sct_itf<def_lv5_sct_policy>;

def_lv3_sct_policy::worker_num_type def_lv3_sct_policy::get_num_workers() noexcept {
    const auto& sched = def_lv5_sct_itf::scheduler::get_instance();
    return sched.get_num_workers();
}

// level 6: mutex, cv

struct def_lv6_sct_policy {
    using lv5_itf_type = def_lv5_sct_itf;
    
    template <typename Pool>
    static fdn::size_t get_mcs_mutex_pool_threshold(Pool& /*pool*/) noexcept {
        return constants::default_mcs_pool_threshold;
    }
};

using def_lv6_sct_itf = lv6_sct_itf<def_lv6_sct_policy>;

// level 7: barrier

struct def_lv7_sct_policy {
    using lv6_itf_type = def_lv6_sct_itf;
};

using def_lv7_sct_itf = lv7_sct_itf<def_lv7_sct_policy>;


using def_sct_itf = def_lv7_sct_itf;

template <>
struct get_ult_itf_type<ult_tag_t::SCT>
    : fdn::type_identity<def_sct_itf> { };

} // namespace cmpth

