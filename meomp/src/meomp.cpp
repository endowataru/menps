
#include <menps/medsm2/itf/dsm_facade.hpp>
#include "child_worker.hpp"
#include "dist_worker.hpp"
#include "child_worker_group.hpp"
#include "omp_worker.hpp"
#include <menps/mefdn/disable_aslr.hpp>
#include <stdexcept>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <stdarg.h>
#include <menps/meomp.hpp>
#include <menps/mefdn/profiling/time.hpp> // get_current_sec

#include <cmpth/wss/basic_single_worker.hpp>
#include <cmpth/wss/basic_worker_task.hpp>
#include <cmpth/wss/basic_worker_tls.hpp>
#include <cmpth/wss/basic_unique_task_ptr.hpp>
#include <cmpth/sct/sct_call_stack.hpp>
#include <cmpth/sct/sct_continuation.hpp>
#include <cmpth/sct/sct_running_task.hpp>
#include <cmpth/ctx/x86_64_context_policy.hpp>
#include "omp_task_desc.hpp"

extern "C"
int meomp_main(int argc, char** argv);

namespace /*unnamed*/ {

using namespace menps;

using dsm_facade_t = menps::medsm2::dsm_facade;
dsm_facade_t* g_df;

using coll_t = menps::medsm2::dsm_com_itf_t::coll_itf_type;
using space_t = menps::medsm2::mpi_svm_space;

coll_t* g_coll;
space_t* g_sp;

int g_argc;
char** g_argv;

void* g_stack_ptr;
mefdn::size_t g_stack_size;

void* g_heap_ptr;
mefdn::size_t g_heap_size;

} // unnamed namespace

namespace menps {
namespace meomp {

struct my_worker_base_policy;

using my_worker_base = omp_worker<my_worker_base_policy>;

struct my_tss_worker_policy
{
    using derived_type = my_worker_base;
    using base_ult_itf_type = medsm2::dsm_com_itf_t::ult_itf_type;
};

struct my_task_desc_policy
{
    using task_desc_type = task_desc<my_task_desc_policy>;
    
    using context_policy_type = cmpth::x86_64_context_policy<my_worker_base_policy>;
    using context_type =
        typename context_policy_type::template context<my_worker_base*>;
    using transfer_type =
        typename context_policy_type::template transfer<my_worker_base*>;
    
    using assert_policy_type = cmpth::def_assert_policy;
    using log_policy_type = cmpth::def_log_policy;
};

struct my_worker_base_policy
    : my_task_desc_policy
{
    using derived_type = my_worker_base;
    
    using single_worker_type = cmpth::basic_single_worker<my_worker_base_policy>;
    using worker_task_type = cmpth::basic_worker_task<my_worker_base_policy>;
    using worker_tls_type = cmpth::basic_worker_tls<my_worker_base_policy>;
    
    using task_desc_type = task_desc<my_task_desc_policy>;
    using call_stack_type = cmpth::sct_call_stack<my_worker_base_policy>;
    using continuation_type = cmpth::sct_continuation<my_worker_base_policy>;
    using running_task_type = cmpth::sct_running_task<my_worker_base_policy>;
    
    using base_ult_itf_type = my_tss_worker_policy::base_ult_itf_type;
    using task_ref_type = task_ref<my_task_desc_policy>;
    using unique_task_ptr_type = cmpth::basic_unique_task_ptr<my_worker_base_policy>;
    
    enum class cmd_code_type {
        none = 0
    ,   barrier
    ,   start_parallel
    ,   end_parallel
    ,   exit_parallel
    ,   exit_program
    };
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
    struct cmd_info_type {
        cmd_code_type   code;
        omp_func_type   func;
        omp_data_type   data;
    };
    
    static space_t& get_dsm_space() noexcept { return *g_sp; }
};

class my_child_worker;
class my_child_worker_group;
class my_dist_worker;

struct my_child_worker_policy
{
    using derived_type = my_child_worker;
    using worker_base_type = my_worker_base;
    
    using worker_group_type = my_child_worker_group;
    
    using cmd_info_type = my_worker_base_policy::cmd_info_type;
    using cmd_code_type = my_worker_base_policy::cmd_code_type;
    
    static void fatal_error() {
        throw std::logic_error("Fatal error in child worker");
    }
};

inline my_worker_base_policy::call_stack_type make_call_stack_at(my_worker_base_policy::task_desc_type* const desc, mefdn::size_t id) {
    desc->ctx = my_worker_base_policy::context_type();
    
    const auto ptr = static_cast<mefdn::byte*>(g_stack_ptr);
    desc->stk_top = ptr + g_stack_size*id;
    desc->stk_bottom = ptr + g_stack_size*(id+1);
    
    return my_worker_base_policy::call_stack_type{ my_worker_base_policy::unique_task_ptr_type{desc} };
}

class my_child_worker
    : public my_worker_base
    , public child_worker<my_child_worker_policy>
{
    using task_desc_type = my_worker_base_policy::task_desc_type;
    
public:
    void set_call_stack() {
        this->set_work_stack(make_call_stack_at(&desc_, 1 + this->get_thread_num()));
    }
    
private:
    task_desc_type desc_;
};



struct my_child_worker_group_policy
{
    using derived_type = my_child_worker_group;
    using child_worker_type = my_child_worker;
    
    using comm_ult_itf_type = medsm2::dsm_com_itf_t::ult_itf_type;
    
    using worker_base_type = my_worker_base;
};


class my_child_worker_group
    : public child_worker_group<my_child_worker_group_policy>
{
public:
    space_t& get_dsm_space() {
        return *g_sp;
    }
};

struct my_dist_worker_policy
{
    using derived_type = my_dist_worker;
    using worker_base_type = my_worker_base;
    
    using cmd_info_type = my_worker_base_policy::cmd_info_type;
    using cmd_code_type = my_worker_base_policy::cmd_code_type;
    
    static void fatal_error() {
        throw std::logic_error("Fatal error in distributed worker");
    }
};

class my_dist_worker
    : public my_worker_base
    , public dist_worker<my_dist_worker_policy>
{
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
    using task_desc_type = my_worker_base_policy::task_desc_type;
    
public:
    my_dist_worker() {
        this->set_work_stack(make_call_stack_at(&desc_, 0));
    }
    ~my_dist_worker() {
        this->reset_work_stack();
    }
    
    coll_t& get_comm_coll() {
        return *g_coll;
    }
    
    void call_entrypoint() {
        meomp_main(g_argc, g_argv);
    }
    
    space_t& get_dsm_space() {
        return *g_sp;
    }
    
    void start_parallel_on_children(
        const omp_func_type func
    ,   const omp_data_type data
    ,   const int           total_num_threads
    ,   const int           thread_num_first
    ,   const int           num_threads
    ,   const bool          is_master_node
    ) {
        this->wg_.start_parallel(func, data, total_num_threads, thread_num_first, num_threads, is_master_node);
    }
    void end_parallel_on_children() {
        this->wg_.end_parallel();
    }
    
    void barrier_on_master()
    {
        this->wg_.barrier_on(*this);
    }
    
private:
    my_child_worker_group   wg_;
    task_desc_type          desc_;
};

} // namespace meomp
} // namespace menps

using worker_base_type = menps::meomp::my_worker_base;
using child_worker_type = menps::meomp::my_child_worker;

extern "C"
int omp_get_thread_num();
extern "C"
int omp_get_thread_num()
{
    return worker_base_type::get_cur_worker().get_thread_num();
}

extern "C"
int omp_get_thread_num_() {
    return omp_get_thread_num();
}

extern "C"
int omp_get_num_threads();
extern "C"
int omp_get_num_threads()
{
    return worker_base_type::get_cur_worker().get_num_threads();
}

extern "C"
int omp_get_num_threads_() { return omp_get_num_threads(); }

extern "C"
int omp_get_max_threads();
extern "C"
int omp_get_max_threads()
{
    return g_coll->get_num_procs() * meomp::my_dist_worker::get_threads_per_proc();
}

extern "C"
int omp_get_max_threads_() { return omp_get_max_threads(); }

extern "C"
int meomp_get_num_procs() {
    return g_coll->get_num_procs();
}

extern "C"
int meomp_get_proc_num() {
    return g_coll->this_proc_id();
}

extern "C"
int meomp_get_local_thread_num() {
    return omp_get_thread_num() % meomp::my_dist_worker::get_threads_per_proc();
}

extern "C"
double omp_get_wtime()
{
    return menps::mefdn::get_current_sec();
}

extern "C"
double omp_get_wtime_() {
    return omp_get_wtime();
}

extern "C"
void GOMP_barrier();
extern "C"
void GOMP_barrier()
{
    worker_base_type::get_cur_worker().barrier();
}

namespace /*unnamed*/ {

meomp::my_dist_worker* g_dw;

} // unnamed namespace

extern "C"
void GOMP_parallel_start(
    void (* const func)(void*)
,   void* const data
,   const unsigned int num_threads
);
extern "C"
void GOMP_parallel_start(
    void (* const func)(void*)
,   void* const data
,   const unsigned int /*num_threads*/ // TODO
) {
    g_dw->start_parallel(func, data);
}

extern "C"
void GOMP_parallel_end();
extern "C"
void GOMP_parallel_end()
{
    g_dw->end_parallel();
}

extern "C"
void GOMP_parallel(
    void (* const func)(void*)
,   void* const data
,   const unsigned int num_threads
,   const unsigned int /*flags*/
);
extern "C"
void GOMP_parallel(
    void (* const func)(void*)
,   void* const data
,   const unsigned int num_threads
,   const unsigned int /*flags*/
) {
    GOMP_parallel_start(func, data, num_threads);
    func(data);
    GOMP_parallel_end();
}

#define MEOMP_USE_MEDSM_MUTEX

// Just copy the definition from libgomp omp.h.
typedef struct {
    unsigned char _x[4]
    __attribute__((__aligned__(4)));
}
omp_lock_t;

extern "C"
void omp_set_lock(omp_lock_t* const lk);
extern "C"
void omp_set_lock(omp_lock_t* const lk)
{
    #ifdef MEOMP_USE_MEDSM_MUTEX
    const auto mtx_id = *reinterpret_cast<space_t::mutex_id_t*>(lk);
    g_sp->lock_mutex(mtx_id);
    
    #else
    auto* const target =
        reinterpret_cast<mefdn::uint32_t*>(lk);
    
    while (true)
    {
        mefdn::uint32_t expected = 0;
        mefdn::uint32_t desired = 1;
        
        if (g_sp->compare_exchange_strong_acquire(*target, expected, desired)) {
            break;
        }
    }
    #endif
}
extern "C"
void omp_unset_lock(omp_lock_t* const lk);
extern "C"
void omp_unset_lock(omp_lock_t* const lk)
{
    #ifdef MEOMP_USE_MEDSM_MUTEX
    const auto mtx_id = *reinterpret_cast<space_t::mutex_id_t*>(lk);
    g_sp->unlock_mutex(mtx_id);
    
    #else
    auto* const target =
        reinterpret_cast<mefdn::uint32_t*>(lk);
    
    g_sp->store_release(target, 0);
    #endif
}
extern "C"
void omp_init_lock(omp_lock_t* const lk);
extern "C"
void omp_init_lock(omp_lock_t* const lk) {
    #ifdef MEOMP_USE_MEDSM_MUTEX
    const auto mtx_id_ptr = reinterpret_cast<space_t::mutex_id_t*>(lk);
    *mtx_id_ptr = g_sp->allocate_mutex();
    
    #else
    auto* const target =
        reinterpret_cast<mefdn::uint32_t*>(lk);
    
    *target = 0;
    #endif
}
extern "C"
void omp_destroy_lock(omp_lock_t* /*lk*/);
extern "C"
void omp_destroy_lock(omp_lock_t* const lk) {
    #ifdef MEOMP_USE_MEDSM_MUTEX
    const auto mtx_id = *reinterpret_cast<space_t::mutex_id_t*>(lk);
    g_sp->deallocate_mutex(mtx_id);
    
    #else
    // Do nothing.
    #endif
}

namespace /*unnamed*/ {

omp_lock_t* g_critical_lock;

} // unnamed namespace

extern "C"
void GOMP_critical_start();
extern "C"
void GOMP_critical_start() {
    omp_set_lock(g_critical_lock);
}

extern "C"
void GOMP_critical_end();
extern "C"
void GOMP_critical_end() {
    omp_unset_lock(g_critical_lock);
}

extern "C"
bool GOMP_single_start(void) {
    return worker_base_type::get_cur_worker().get_thread_num() == 0;
}

// LLVM

struct ident;

using kmp_int32 = mefdn::int32_t;
using kmp_int64 = mefdn::int64_t;

using kmpc_micro = void (*)(kmp_int32* , kmp_int32*, ...);
using microtask_t = void (*)(int*, int*, ...);

namespace menps {
namespace meomp {

constexpr mefdn::size_t max_num_kmp_args = 15;

struct kmp_invoker
{
    kmpc_micro      func;
    mefdn::size_t   argc;
    void*           ptrs[max_num_kmp_args];
    
    static void handler(void* const self_ptr)
    {
        MEFDN_ASSERT(self_ptr != nullptr);
        auto& self = *static_cast<kmp_invoker*>(self_ptr);
        
        const auto thread_num = worker_base_type::get_cur_worker().get_thread_num();
        kmp_int32 gtid = thread_num;
        kmp_int32 btid = thread_num;
        
        switch (self.argc) {
            case 0:
                self.func(&gtid, &btid);
                break;
            
            case 1:
                self.func(&gtid, &btid, self.ptrs[0]);
                break;
            
            case 2:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1]);
                break;
            
            case 3:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2]);
                break;
            
            case 4:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3]);
                break;
            
            case 5:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4]);
                break;
            
            case 6:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5]);
                break;
            
            case 7:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5], self.ptrs[6]);
                break;
            
            case 8:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5], self.ptrs[6], self.ptrs[7]);
                break;
            
            case 9:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5], self.ptrs[6], self.ptrs[7], self.ptrs[8]);
                break;
            
            case 10:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5], self.ptrs[6], self.ptrs[7], self.ptrs[8], self.ptrs[9]);
                break;
            
            case 11:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5], self.ptrs[6], self.ptrs[7], self.ptrs[8], self.ptrs[9], self.ptrs[10]);
                break;
            
            case 12:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5], self.ptrs[6], self.ptrs[7], self.ptrs[8], self.ptrs[9], self.ptrs[10], self.ptrs[11]);
                break;
            
            case 13:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5], self.ptrs[6], self.ptrs[7], self.ptrs[8], self.ptrs[9], self.ptrs[10], self.ptrs[11], self.ptrs[12]);
                break;
            
            case 14:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5], self.ptrs[6], self.ptrs[7], self.ptrs[8], self.ptrs[9], self.ptrs[10], self.ptrs[11], self.ptrs[12], self.ptrs[13]);
                break;
            
            case 15:
                self.func(&gtid, &btid, self.ptrs[0], self.ptrs[1], self.ptrs[2], self.ptrs[3], self.ptrs[4], self.ptrs[5], self.ptrs[6], self.ptrs[7], self.ptrs[8], self.ptrs[9], self.ptrs[10], self.ptrs[11], self.ptrs[12], self.ptrs[13], self.ptrs[14]);
                break;
            
            default:
                throw std::runtime_error("Too many arguments: " + std::to_string(self.argc));
                break;
        }
    }
};

} // namespace meomp
} // namespace menps

extern "C"
void __kmpc_fork_call(ident* /*id*/, const kmp_int32 argc, const kmpc_micro microtask, ...);
extern "C"
void __kmpc_fork_call(ident* /*id*/, const kmp_int32 argc, const kmpc_micro microtask, ...) {
    using menps::meomp::kmp_invoker;
    
    kmp_invoker inv = kmp_invoker();
    inv.func = microtask;
    inv.argc = argc;
    
    {
        va_list ap;
        va_start(ap, argc);
        for (int i = 0; i < argc; ++i) {
            inv.ptrs[i] = va_arg(ap, void*);
        }
        va_end(ap);
    }
    
    g_dw->start_parallel(kmp_invoker::handler, &inv);
    
    kmp_invoker::handler(&inv);
    
    g_dw->end_parallel();
}


extern "C"
kmp_int32 __kmpc_global_thread_num(ident* id);
extern "C"
kmp_int32 __kmpc_global_thread_num(ident* /*id*/) {
    return worker_base_type::get_cur_worker().get_thread_num();
}

extern "C"
kmp_int32 __kmpc_global_num_threads(ident* id);
extern "C"
kmp_int32 __kmpc_global_num_threads(ident* /*id*/) {
    return worker_base_type::get_cur_worker().get_num_threads();
}

extern "C"
void __kmpc_barrier(ident* id, kmp_int32 global_tid);
extern "C"
void __kmpc_barrier(ident* /*id*/, kmp_int32 /*global_tid*/) {
    worker_base_type::get_cur_worker().barrier();
}

extern "C"
kmp_int32 __kmpc_ok_to_fork(ident* id);
extern "C"
kmp_int32 __kmpc_ok_to_fork(ident* /*id*/) {
    // Always returns TRUE as the official documentation says.
    return 1;
}

extern "C"
void __kmpc_serialized_parallel(ident* id, kmp_int32 global_tid);
extern "C"
void __kmpc_serialized_parallel(ident* /*id*/, kmp_int32 /*global_tid*/) {
    // Do nothing.
}

extern "C"
void __kmpc_end_serialized_parallel(ident* id, kmp_int32 global_tid);
extern "C"
void __kmpc_end_serialized_parallel(ident* /*id*/, kmp_int32 /*global_tid*/) {
    // Do nothing.
}


extern "C"
kmp_int32 __kmpc_master(ident* loc, kmp_int32 global_tid);
extern "C"
kmp_int32 __kmpc_master(ident* /*loc*/, kmp_int32 /*global_tid*/) {
    return worker_base_type::get_cur_worker().get_thread_num() == 0;
}
extern "C"
void __kmpc_end_master(ident* loc, kmp_int32 global_tid);
extern "C"
void __kmpc_end_master(ident* /*loc*/, kmp_int32 /*global_tid*/) {
    // Do nothing.
}

extern "C"
kmp_int32 __kmpc_single(ident* loc, kmp_int32 global_tid) {
    // TODO: use atomics for better performance
    return __kmpc_master(loc, global_tid);
}
extern "C"
void __kmpc_end_single(ident* loc, kmp_int32 global_tid) {
    __kmpc_end_master(loc, global_tid);
}

namespace /*unnamed*/ {

template <typename T, typename SignedT>
void kmpc_for_static_init(
    ident* const        /*loc*/
,   const kmp_int32     gtid
,   const kmp_int32     /*schedtype*/
,   kmp_int32* const    plastiter
,   T* const            plower
,   T* const            pupper
,   SignedT* const      pstride
,   const SignedT       incr
,   const SignedT       /*chunk*/
) {
    // See also: kmp_sched.cpp of LLVM OpenMP
    
    MEFDN_STATIC_ASSERT(mefdn::is_signed<SignedT>::value);
    
    const auto& wk = worker_base_type::get_cur_worker();
    const auto num_threads = static_cast<T>(wk.get_num_threads());
    MEFDN_ASSERT(gtid == wk.get_thread_num());
    const auto tid = gtid;
    
    const auto old_lower = *plower;
    const auto old_upper = *pupper;
    
    MEFDN_ASSERT(incr != 0);
    MEFDN_ASSERT((incr > 0) == (old_lower <= old_upper));
    
    const auto trip_count =
            incr > 0
        ?   ((old_upper - old_lower) / incr + 1)
        :   ((old_lower - old_upper) / (-incr) + 1);
    
    // Follow the calculation of kmp_sch_static_greedy.
    
    const auto big_chunk_inc_count =
        mefdn::roundup_divide(trip_count, num_threads);
    
    const auto new_lower = old_lower + tid * big_chunk_inc_count;
    const auto new_upper = new_lower + big_chunk_inc_count - incr;
    
    const auto new_last_iter =
            incr > 0
        ?   (new_lower <= old_upper && new_upper > old_upper - incr)
        :   (new_lower >= old_upper && new_upper < old_upper - incr);
    
    *plower = new_lower;
    *pupper = new_upper;
    *plastiter = new_last_iter;
    *pstride = trip_count;
}

} // unnamed namespace

extern "C"
void __kmpc_for_static_init_4(
    ident* const        loc
,   const kmp_int32     gtid
,   const kmp_int32     schedtype
,   kmp_int32* const    plastiter
,   kmp_int32* const    plower
,   kmp_int32* const    pupper
,   kmp_int32* const    pstride
,   const kmp_int32     incr
,   const kmp_int32     chunk
);
extern "C"
void __kmpc_for_static_init_4(
    ident* const        loc
,   const kmp_int32     gtid
,   const kmp_int32     schedtype
,   kmp_int32* const    plastiter
,   kmp_int32* const    plower
,   kmp_int32* const    pupper
,   kmp_int32* const    pstride
,   const kmp_int32     incr
,   const kmp_int32     chunk
) {
    kmpc_for_static_init(loc, gtid, schedtype, plastiter, plower, pupper, pstride, incr, chunk);
}


extern "C"
void __kmpc_for_static_init_8(
    ident* const        loc
,   const kmp_int32     gtid
,   const kmp_int32     schedtype
,   kmp_int32* const    plastiter
,   kmp_int64* const    plower
,   kmp_int64* const    pupper
,   kmp_int64* const    pstride
,   const kmp_int64     incr
,   const kmp_int64     chunk
) {
    kmpc_for_static_init(loc, gtid, schedtype, plastiter, plower, pupper, pstride, incr, chunk);
}


extern "C"
void __kmpc_for_static_fini(ident* loc, kmp_int32 global_tid);
extern "C"
void __kmpc_for_static_fini(ident* /*loc*/, kmp_int32 /*global_tid*/) {
    // Do nothing.
}

extern "C"
void __kmpc_init_lock(omp_lock_t* const lk)
{
    omp_init_lock(lk);
}
extern "C"
void __kmpc_destroy_lock(omp_lock_t* const lk)
{
    omp_destroy_lock(lk);
}

extern "C"
void __kmpc_set_lock(omp_lock_t* const lk);
extern "C"
void __kmpc_set_lock(omp_lock_t* const lk)
{
    omp_set_lock(lk);
}
extern "C"
void __kmpc_unset_lock(omp_lock_t* const lk);
extern "C"
void __kmpc_unset_lock(omp_lock_t* const lk)
{
    omp_unset_lock(lk);
}

extern "C"
void __kmpc_critical();
extern "C"
void __kmpc_critical() {
    GOMP_critical_start();
}

extern "C"
void __kmpc_end_critical();
extern "C"
void __kmpc_end_critical() {
    GOMP_critical_end();
}

extern "C" {

void* meomp_malloc(const menps::mefdn::size_t size_in_bytes) {
    return g_df->allocate(size_in_bytes);
}
void meomp_free(void* const ptr) {
    g_df->deallocate(ptr);
}

}

#ifndef MEFDN_OS_MAC_OS_X
extern "C" {

extern void* _dsm_data_begin;
extern void* _dsm_data_end;

} // extern "C"
#endif

namespace /*unnamed*/ {

inline char** pack_argv(int argc, char* argv[])
{
    mefdn::size_t size = sizeof(char*)*argc;
    for (int i = 0; i < argc; ++i) {
        size += std::strlen(argv[i])+1;
    }
    
    const auto ptr = meomp_malloc(size);
    const auto global_argv = static_cast<char**>(ptr);
    auto str_buf = reinterpret_cast<char*>(&global_argv[argc]);
    
    for (int i = 0; i < argc; ++i) {
        global_argv[i] = str_buf;
        std::strcpy(str_buf, argv[i]);
        str_buf += std::strlen(argv[i])+1;
    }
    
    MEFDN_ASSERT(str_buf - static_cast<char*>(ptr) == size);
    
    return global_argv;
}

inline mefdn::size_t get_stack_size()
{
    mefdn::size_t ret = MEOMP_DEF_STKSIZE;
    if (const auto str = std::getenv("MEOMP_DEF_STKSIZE")) {
        ret = static_cast<mefdn::size_t>(std::atoi(str));
    }
    if (ret % 0x1000 != 0 || ret <= 0) {
        throw std::invalid_argument("Invalid MEOMP_DEF_STKSIZE");
    }
    return ret;
}

inline mefdn::size_t get_heap_size()
{
    mefdn::size_t ret = MEOMP_HEAP_SIZE;
    if (const auto str = std::getenv("MEOMP_HEAP_SIZE")) {
        ret = static_cast<mefdn::size_t>(std::atoi(str));
    }
    if (ret % 0x1000 != 0 || ret <= 0) {
        throw std::invalid_argument("Invalid MEOMP_HEAP_SIZE");
    }
    return ret;
}

} // unnamed namespace

int main(int argc, char* argv[])
{
    using namespace menps;
    
    mefdn::disable_aslr(argc, argv);
    
    dsm_facade_t df{&argc, &argv};
    g_df = &df;
    
    auto& sp = df.get_space();
    g_sp = &sp;
    
    auto& coll = df.get_com_itf().get_coll();
    g_coll = &coll;
    
    #if 0
    using medsm2::sigsegv_catcher;
    auto segv_catch =
        mefdn::make_unique<sigsegv_catcher>(
            sigsegv_catcher::config{
                [&g_sp] (void* const ptr) {
                    const auto wk_ptr =
                        meomp::my_worker_base::get_current_worker_ptr();
                    
                    if (wk_ptr != nullptr) {
                        return wk_ptr->try_upgrade(ptr);
                    }
                    else {
                        // Immediately upgrade.
                        // Used in initialization of global variables.
                        return g_sp->try_upgrade(ptr);
                    }
                }
            ,   false
            }
        );
    #endif
    
    #ifndef MEFDN_OS_MAC_OS_X
    df.init_global_var_seg(&_dsm_data_begin, &_dsm_data_end, MEDSM2_GLOBAL_VAR_BLOCK_SIZE);
    #endif
    
    const auto num_procs = coll.get_num_procs();
    
    const mefdn::size_t stack_size = get_stack_size();
    
    const auto stack_ptr_start =
        sp.coll_alloc_seg((num_procs*meomp::my_dist_worker::get_threads_per_proc()+1) * stack_size, stack_size);
    
    g_stack_ptr = stack_ptr_start;
    g_stack_size = stack_size;
    
    g_heap_size = get_heap_size();
    g_heap_ptr = df.init_heap_seg(g_heap_size, MEOMP_HEAP_BLOCK_SIZE);
    
    if (coll.this_proc_id() == 0) {
        g_critical_lock = static_cast<omp_lock_t*>(df.allocate(sizeof(omp_lock_t)));
    }
    
    g_coll->untyped_broadcast(0, &g_critical_lock, sizeof(omp_lock_t*));
    
    if (coll.this_proc_id() == 0) {
        sp.enable_on_this_thread();
        
        omp_init_lock(g_critical_lock);
        
        g_argc = argc;
        g_argv = pack_argv(argc, argv);
        
        sp.disable_on_this_thread();
    }
    
    meomp::my_dist_worker dw;
    g_dw = &dw;
    
    sp.start_release_thread();
    
    dw.execute_loop();
    
    // Do a barrier before exiting.
    g_coll->barrier();
    
    sp.stop_release_thread();
    
    df.print_prof();
    
    if (coll.this_proc_id() == 0) {
        sp.enable_on_this_thread();
        
        meomp_free(g_argv);
        
        omp_destroy_lock(g_critical_lock);
        
        sp.disable_on_this_thread();
    }
    
    // Do a barrier before destroying the resources.
    g_coll->barrier();
    
    return 0;
}

