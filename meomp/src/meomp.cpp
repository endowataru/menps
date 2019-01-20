
#include <menps/medsm2/svm/mpi_svm_space.hpp>
#include <menps/mectx/generic/single_ult_worker.hpp>
#include <menps/mectx/generic/thread_specific_worker.hpp>
#include "child_worker.hpp"
#include "dist_worker.hpp"
#include "child_worker_group.hpp"
#include "omp_worker.hpp"
#include <menps/mectx/context_policy.hpp>
#include <menps/mefdn/disable_aslr.hpp>
#include <menps/mefdn/profiling/clock.hpp> // get_cpu_clock
#include <menps/mefdn/thread/barrier.hpp>
#include <stdexcept>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <stdarg.h>
#include <menps/meomp.hpp>
#include <menps/medsm2/prof.hpp>
#include <menps/mefdn/profiling/time.hpp>

#ifdef MEOMP_SEPARATE_WORKER_THREAD
#include <menps/medsm2/svm/sigsegv_catcher.hpp>
#endif

#if 0
extern void* g_watch_ptr;
#endif

extern "C"
int meomp_main(int argc, char** argv);

namespace /*unnamed*/ {

using namespace menps;

using coll_t = menps::medsm2::dsm_com_creator::dsm_com_itf_type::coll_itf_type;
using space_t = menps::medsm2::mpi_svm_space;

coll_t* g_coll;
space_t* g_sp;

int g_argc;
char** g_argv;

void* g_stack_ptr;
mefdn::size_t g_stack_size;

void* g_heap_ptr;
mefdn::size_t g_heap_size;

#if 0
struct get_state
{
    std::string operator() ()
    {
        fmt::MemoryWriter w;
        w.write(
            "proc:{}\tthread:{:x}\tult:{:x}\tlog_id:{}\tclock:{}\t"
        ,   g_coll->this_proc_id()
        ,   reinterpret_cast<mefdn::uintptr_t>(pthread_self())
            // TODO: use mefdn::this_thread::get_id()
        ,   reinterpret_cast<mefdn::uintptr_t>(
                medsm2::dsm_com_creator::ult_itf_type::this_thread::native_handle()
            )
        ,   this->number_++
        ,   mefdn::get_cpu_clock() // TODO
        );
        #if 0
        if (g_watch_ptr != nullptr)
            w.write("val:0x{:x}\t", *(uint64_t*)g_watch_ptr);
        #endif
        
        return w.str();
    }
    
private:
    mefdn::size_t number_ = 0;
};
#endif

} // unnamed namespace

namespace menps {
namespace meomp {

struct my_worker_base_policy;

using my_worker_base = omp_worker<my_worker_base_policy>;

using context_t = mectx::context<my_worker_base*>;
using transfer_t = mectx::transfer<my_worker_base*>;

class my_omp_ult_ref;

struct my_tss_worker_policy
{
    using derived_type = my_worker_base;
    #ifdef MEOMP_SEPARATE_WORKER_THREAD
    // Kernel threads are used in order to manage
    // signal handlers separately for each OpenMP worker thread.
    using base_ult_itf_type = meult::klt_policy;
    #else
    using base_ult_itf_type = medsm2::dsm_com_creator::ult_itf_type;
    #endif
};

struct my_worker_base_policy
{
    using derived_type = my_worker_base;
    
    using single_ult_worker_type = mectx::single_ult_worker<my_worker_base_policy>;
    using ult_switcher_type = mectx::ult_switcher<my_worker_base_policy>;
    using thread_specific_worker_type = mectx::thread_specific_worker<my_tss_worker_policy>;
    using context_policy_type = mectx::context_policy;
    
    using ult_ref_type = my_omp_ult_ref;
    
    using worker_ult_itf_type = my_tss_worker_policy::base_ult_itf_type;
    
    using context_type = context_t;
    using transfer_type = transfer_t;
    
    enum class cmd_code_type {
        none = 0
    ,   barrier
    ,   start_parallel
    ,   end_parallel
    ,   exit_parallel
    ,   exit_program
    #ifdef MEOMP_SEPARATE_WORKER_THREAD
    ,   try_upgrade
    #endif
    };
    
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
    struct cmd_info_type {
        cmd_code_type   code;
        omp_func_type   func;
        omp_data_type   data;
    };
};

class my_omp_ult_ref
{
public:
    my_omp_ult_ref() noexcept = default;
    
    explicit my_omp_ult_ref(const int id)
        : id_(id)
    {
        MEFDN_ASSERT(id_ >= 0);
    }
    
    static my_omp_ult_ref make_root() {
        my_omp_ult_ref ret;
        ret.id_ = -1;
        return ret;
    }
    
    bool is_valid() const noexcept {
        return id_ >= -1;
    }
    
    context_t get_context() { return ctx_; }
    void set_context(context_t ctx) { ctx_ = ctx; }
    
    void* get_stack_ptr() {
        const auto ptr = static_cast<mefdn::byte*>(g_stack_ptr);
        return ptr + g_stack_size*(id_+1);
    }
    mefdn::size_t get_stack_size() {
        return g_stack_size;
    }
    
    void pin() {
        MEFDN_ASSERT(this->is_valid());
        if (id_ != -1) {
            g_sp->pin(get_stack_ptr_bottom(), get_stack_size());
        }
    }
    void unpin() {
        MEFDN_ASSERT(this->is_valid());
        if (id_ != -1) {
            g_sp->unpin(get_stack_ptr_bottom(), get_stack_size());
        }
    }
    
private:
    void* get_stack_ptr_bottom() {
        const auto ptr = static_cast<mefdn::byte*>(g_stack_ptr);
        return ptr + g_stack_size*id_;
    }
    
    int id_ = -2;
        // -2: invalid
        // -1: root
        // >=0: work
    context_t ctx_ = context_t();
};

class my_child_worker;
class my_child_worker_group;
class my_dist_worker;

struct my_child_worker_policy
{
    using derived_type = my_child_worker;
    
    using ult_ref_type = my_omp_ult_ref;
    
    using worker_group_type = my_child_worker_group;
    
    using cmd_info_type = my_worker_base_policy::cmd_info_type;
    using cmd_code_type = my_worker_base_policy::cmd_code_type;
    
    static void fatal_error() {
        throw std::logic_error("Fatal error in child worker");
    }
};

class my_child_worker
    : public my_worker_base
    , public child_worker<my_child_worker_policy>
{
    using ult_ref_type = typename my_child_worker_policy::ult_ref_type;
    
public:
    ult_ref_type make_work_ult() {
        return ult_ref_type(1 + this->get_thread_num());
    }
};



struct my_child_worker_group_policy
{
    using derived_type = my_child_worker_group;
    using child_worker_type = my_child_worker;
    
    using comm_ult_itf_type = medsm2::dsm_com_creator::ult_itf_type;
    
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
    
    using cmd_info_type = my_worker_base_policy::cmd_info_type;
    using cmd_code_type = my_worker_base_policy::cmd_code_type;
    
    using ult_ref_type = my_omp_ult_ref;
    
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
    
    using ult_ref_type = typename my_dist_worker_policy::ult_ref_type;
    
public:
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
    
    static ult_ref_type make_work_ult() {
        return ult_ref_type(0);
    }
    
private:
    my_child_worker_group wg_;
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
    return worker_base_type::get_current_worker().get_thread_num();
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
    return worker_base_type::get_current_worker().get_num_threads();
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
    worker_base_type::get_current_worker().barrier();
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

MEOMP_GLOBAL_VAR omp_lock_t g_critical_lock;

MEOMP_GLOBAL_VAR omp_lock_t g_heap_lock;
#ifdef MEOMP_ENABLE_LINEAR_ALLOCATOR
MEOMP_GLOBAL_VAR mefdn::size_t g_heap_used;
#else
MEOMP_GLOBAL_VAR mspace g_heap_ms;
#endif

} // unnamed namespace

extern "C"
void GOMP_critical_start();
extern "C"
void GOMP_critical_start() {
    omp_set_lock(&g_critical_lock);
}

extern "C"
void GOMP_critical_end();
extern "C"
void GOMP_critical_end() {
    omp_unset_lock(&g_critical_lock);
}



// LLVM

struct ident;

using kmp_int32 = mefdn::int32_t;

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
        
        const auto thread_num = worker_base_type::get_current_worker().get_thread_num();
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
    return worker_base_type::get_current_worker().get_thread_num();
}

extern "C"
kmp_int32 __kmpc_global_num_threads(ident* id);
extern "C"
kmp_int32 __kmpc_global_num_threads(ident* /*id*/) {
    return worker_base_type::get_current_worker().get_num_threads();
}

extern "C"
void __kmpc_barrier(ident* id, kmp_int32 global_tid);
extern "C"
void __kmpc_barrier(ident* /*id*/, kmp_int32 /*global_tid*/) {
    worker_base_type::get_current_worker().barrier();
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
    return worker_base_type::get_current_worker().get_thread_num() == 0;
}
extern "C"
void __kmpc_end_master(ident* loc, kmp_int32 global_tid);
extern "C"
void __kmpc_end_master(ident* /*loc*/, kmp_int32 /*global_tid*/) {
    // Do nothing.
}

namespace /*unnamed*/ {

template <typename T, typename SignedT>
void kmpc_for_static_init_4(
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
    
    const auto& wk = worker_base_type::get_current_worker();
    const auto num_threads = wk.get_num_threads();
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
    kmpc_for_static_init_4(loc, gtid, schedtype, plastiter, plower, pupper, pstride, incr, chunk);
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
    omp_set_lock(&g_heap_lock);
    #ifdef MEOMP_ENABLE_LINEAR_ALLOCATOR
    const auto ptr = static_cast<mefdn::byte*>(g_heap_ptr) + g_heap_used;
    // Fix alignment.
    g_heap_used += mefdn::roundup_divide(size_in_bytes, 16ul) * 16ul; // TODO: avoid magic number
    if (g_heap_used >= g_heap_size) {
        throw std::bad_alloc();
    }
    #else
    void* ptr = mspace_malloc(g_heap_ms, size_in_bytes);
    #endif
    omp_unset_lock(&g_heap_lock);
    return ptr;
}
void meomp_free(void* const ptr) {
    #ifndef MEOMP_ENABLE_LINEAR_ALLOCATOR
    omp_set_lock(&g_heap_lock);
    mspace_free(g_heap_ms, ptr);
    omp_unset_lock(&g_heap_lock);
    #endif
}

}

extern "C" {

extern void* _dsm_data_begin;
extern void* _dsm_data_end;

} // extern "C"

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

} // unnamed namespace

int main(int argc, char* argv[])
{
    using namespace menps;
    
    mefdn::disable_aslr(argc, argv);
    
    medsm2::dsm_com_creator cc(&argc, &argv);
    auto& com = cc.get_dsm_com_itf();
    auto& coll = com.get_coll();
    g_coll = &coll;
    
    #if 0
    mefdn::logger::set_state_callback(get_state{});
    #endif
    
    medsm2::mpi_svm_space sp(com);
    
    g_sp = &sp;
    
    #ifdef MEOMP_SEPARATE_WORKER_THREAD
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
    
    {
        const auto global_var_blk_size = MEDSM2_GLOBAL_VAR_BLOCK_SIZE;
        
        const auto data_begin = reinterpret_cast<mefdn::byte*>(&_dsm_data_begin);
        const auto data_end   = reinterpret_cast<mefdn::byte*>(&_dsm_data_end);
        
        MEFDN_ASSERT(reinterpret_cast<mefdn::uintptr_t>(data_begin) % global_var_blk_size == 0);
        
        const auto data_size = data_end - data_begin;
        
        if (data_size > 0) {
            MEFDN_LOG_VERBOSE(
                "msg:Initialize global variables.\t"
                "data_begin:0x{:x}\t"
                "data_end:0x{:x}\t"
                "data_size:0x{:x}\t"
            ,   reinterpret_cast<mefdn::uintptr_t>(data_begin)
            ,   reinterpret_cast<mefdn::uintptr_t>(data_end)
            ,   data_size
            );
            
            sp.coll_alloc_global_var_seg(data_size, global_var_blk_size, data_begin);
        }
    }
    
    const auto num_procs = coll.get_num_procs();
    
    const mefdn::size_t stack_size = get_stack_size();
    
    const auto stack_ptr_start =
        sp.coll_alloc_seg((num_procs*meomp::my_dist_worker::get_threads_per_proc()+1) * stack_size, stack_size);
    
    g_stack_ptr = stack_ptr_start;
    g_stack_size = stack_size;
    
    g_heap_size = MEOMP_HEAP_SIZE;
    g_heap_ptr =
        sp.coll_alloc_seg(g_heap_size, MEOMP_HEAP_BLOCK_SIZE);
    
    if (coll.this_proc_id() == 0) {
        sp.enable_on_this_thread();
        
        #ifndef MEOMP_ENABLE_LINEAR_ALLOCATOR
        g_heap_ms = create_mspace_with_base(g_heap_ptr, MEOMP_HEAP_SIZE, 1);
        #endif
        
        omp_init_lock(&g_critical_lock);
        omp_init_lock(&g_heap_lock);
        
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
    
    #if (defined(MEDSM2_ENABLE_PROF) || defined(MEDEV2_ENABLE_PROF) || defined(MEULT_ENABLE_PROF))
    std::cout << std::flush;
    g_coll->barrier();
    for (coll_t::proc_id_type proc = 0; proc < num_procs; ++proc) {
        if (coll.this_proc_id() == proc) {
            std::cout << fmt::format("- proc: {}\n", proc);
            #ifdef MEDSM2_ENABLE_PROF
            std::cout << medsm2::prof::to_string("    - ");
            #endif
            #ifdef MEDEV2_ENABLE_PROF
            std::cout << medev2::mpi::prof::to_string("    - ");
            #endif
            #ifdef MEULT_ENABLE_PROF
            std::cout << meult::prof::to_string("    - ");
            #endif
            std::cout << std::flush;
        }
        g_coll->barrier();
    }
    #endif
    
    sp.stop_release_thread();
    
    if (coll.this_proc_id() == 0) {
        sp.enable_on_this_thread();
        
        meomp_free(g_argv);
        
        #ifndef MEOMP_ENABLE_LINEAR_ALLOCATOR
        destroy_mspace(g_heap_ms);
        #endif
        
        omp_destroy_lock(&g_heap_lock);
        omp_destroy_lock(&g_critical_lock);
        
        sp.disable_on_this_thread();
    }
    
    // Do a barrier before destroying the resources.
    g_coll->barrier();
    
    return 0;
}

