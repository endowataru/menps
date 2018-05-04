
#include <menps/medsm2/svm/mpi_svm_space.hpp>
#include <menps/mectx/generic/single_ult_worker.hpp>
//#include <menps/mectx/generic/thread_local_worker.hpp>
#include <menps/mectx/generic/thread_specific_worker.hpp>
#include "child_worker.hpp"
#include "dist_worker.hpp"
#include "child_worker_group.hpp"
#include "omp_worker_base.hpp"
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

#if 0
extern void* g_watch_ptr;
#endif

extern "C"
int meomp_main(int argc, char** argv);

namespace /*unnamed*/ {

using namespace menps;

using coll_t = menps::mecom2::mpi_coll;
using space_t = menps::medsm2::mpi_svm_space;

coll_t* g_coll;
space_t* g_sp;

int g_argc;
char** g_argv;

void* g_stack_ptr;
mefdn::size_t g_stack_size;

struct get_state
{
    std::string operator() ()
    {
        fmt::MemoryWriter w;
        w.write(
            "proc:{}\tthread:{:x}\tlog_id:{}\tclock:{}\t"
        ,   g_coll->this_proc_id()
        ,   reinterpret_cast<mefdn::uintptr_t>(pthread_self())
            // TODO: use mefdn::this_thread::get_id()
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

} // unnamed namespace

namespace menps {
namespace meomp {

class my_worker_base;

using context_t = mectx::context<my_worker_base*>;
using transfer_t = mectx::transfer<my_worker_base*>;

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
    context_t ctx_{};
};

struct my_worker_base_policy
{
    using derived_type = my_worker_base;
    using ult_ref_type = my_omp_ult_ref;
    
    using base_ult_itf_type = medsm2::default_ult_itf;
    
    using context_type = context_t;
    using transfer_type = transfer_t;
    
    enum class cmd_code_type {
        none = 0
    ,   barrier
    #ifdef MEOMP_DISABLE_PARALLEL_START
    ,   do_parallel
    #else
    ,   start_parallel
    ,   end_parallel
    #endif
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
};

class my_worker_base
    : public mectx::single_ult_worker<my_worker_base_policy>
    //, public mectx::thread_local_worker<my_worker_base_policy>
    , public mectx::thread_specific_worker<my_worker_base_policy>
    , public mectx::context_policy
    , public omp_worker_base<my_worker_base_policy>
{
    using context_type = my_worker_base_policy::context_type;
    
public:
    void set_work_ult(my_omp_ult_ref work_th) { work_th_ = work_th; }
    
    my_omp_ult_ref& get_root_ult() { return root_th_; }
    my_omp_ult_ref& get_work_ult() { return work_th_; }
    
    void on_before_switch(my_omp_ult_ref& /*from_th*/, my_omp_ult_ref& to_th) {
        to_th.pin();
    }
    void on_after_switch(my_omp_ult_ref& from_th, my_omp_ult_ref& /*to_th*/) {
        from_th.unpin();
    }
    
    context_type get_context(my_omp_ult_ref& th) {
        return th.get_context();
    }
    void set_context(my_omp_ult_ref& th, context_type ctx) {
        th.set_context(ctx);
    }
    
    std::string show_ult_ref(my_omp_ult_ref&) {
        return "";
    }
    void* get_stack_ptr(my_omp_ult_ref& th) {
        return th.get_stack_ptr();
    }
    mefdn::size_t get_stack_size(my_omp_ult_ref& th) {
        return th.get_stack_size();
    }
    
private:
    my_omp_ult_ref root_th_ = my_omp_ult_ref::make_root();
    my_omp_ult_ref work_th_;
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
public:
    // TODO: Replacing the same name is not a good convention.
    void set_thread_num(int num) {
        my_worker_base::set_thread_num(num);
        this->set_work_ult(my_omp_ult_ref(1+num));
    }
};



struct my_child_worker_group_policy
{
    using derived_type = my_child_worker_group;
    using child_worker_type = my_child_worker;
    
    using ult_itf_type = mecom2::default_ult_itf;
    using worker_thread_type = typename ult_itf_type::thread;
    using barrier_type = typename ult_itf_type::barrier;
    
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
    
    static void fatal_error() {
        throw std::logic_error("Fatal error in distributed worker");
    }
};

class my_dist_worker
    : public my_worker_base
    , public dist_worker<my_dist_worker_policy>
    , public mectx::context_policy
{
    using omp_func_type = void (*)(void*);
    using omp_data_type = void*;
    
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
    
    #ifdef MEOMP_DISABLE_PARALLEL_START
    void start_parallel(
        const omp_func_type func
    ,   const omp_data_type data
    ,   const int           total_num_threads
    ,   const int           thread_num_first
    ,   const int           num_threads
    ) {
        my_child_worker_group wg;
        wg.do_parallel(func, data, total_num_threads, thread_num_first, num_threads);
    }
    #else
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
    #endif
    
    void barrier_on_master()
    {
        this->wg_.barrier_on(*this);
    }
    
    void set_work_ult() {
        my_worker_base::set_work_ult(my_omp_ult_ref{0});
    }
    void unset_work_ult() {
        my_worker_base::set_work_ult(my_omp_ult_ref{});
    }
    
    #ifndef MEOMP_DISABLE_PARALLEL_START
private:
    my_child_worker_group wg_;
    #endif
};

} // namespace meomp
} // namespace menps

using worker_base_type = menps::meomp::my_worker_base;
using child_worker_type = menps::meomp::my_child_worker;

extern "C"
int omp_get_thread_num()
{
    return worker_base_type::get_current_worker().get_thread_num();
}

extern "C"
int omp_get_num_threads()
{
    return worker_base_type::get_current_worker().get_num_threads();
}

extern "C"
void GOMP_barrier()
{
    worker_base_type::get_current_worker().barrier();
}

namespace /*unnamed*/ {

meomp::my_dist_worker* g_dw;

} // unnamed namespace

#ifdef MEOMP_DISABLE_PARALLEL_START
extern "C"
void GOMP_parallel(
    void (* const func)(void*)
,   void* const data
,   const unsigned int /*num_threads*/ // TODO
,   const unsigned int /*flags*/
) {
    g_dw->do_parallel(func, data);
}
#else
extern "C"
void GOMP_parallel_start(
    void (* const func)(void*)
,   void* const data
,   const unsigned int /*num_threads*/ // TODO
) {
    g_dw->start_parallel(func, data);
}
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
) {
    GOMP_parallel_start(func, data, num_threads);
    func(data);
    GOMP_parallel_end();
}

// Just copy the definition from libgomp omp.h.
typedef struct {
    unsigned char _x[4]
    __attribute__((__aligned__(4)));
}
omp_lock_t;

extern "C"
void omp_set_lock(omp_lock_t* const lk)
{
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
}
extern "C"
void omp_unset_lock(omp_lock_t* const lk)
{
    auto* const target =
        reinterpret_cast<mefdn::uint32_t*>(lk);
    
    g_sp->store_release(target, 0);
}
extern "C"
void omp_init_lock(omp_lock_t* const lk) {
    auto* const target =
        reinterpret_cast<mefdn::uint32_t*>(lk);
    
    *target = 0;
}
extern "C"
void omp_destroy_lock(omp_lock_t* /*lk*/) {
    // Do nothing.
}

namespace /*unnamed*/ {

MEOMP_GLOBAL_VAR omp_lock_t g_critical_lock;

} // unnamed namespace

extern "C"
void GOMP_critical_start() {
    omp_set_lock(&g_critical_lock);
}

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

constexpr mefdn::size_t max_num_kmp_args = 4;

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
            
            default:
                throw std::runtime_error("Too many arguments");
                break;
        }
    }
};

} // namespace meomp
} // namespace menps

extern "C"
void __kmpc_fork_call(ident* /*id*/, const kmp_int32 argc, const kmpc_micro microtask, ...) {
    using menps::meomp::kmp_invoker;
    
    kmp_invoker inv{};
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
kmp_int32 __kmpc_global_thread_num(ident* /*id*/) {
    return worker_base_type::get_current_worker().get_thread_num();
}

extern "C"
kmp_int32 __kmpc_global_num_threads(ident* /*id*/) {
    return worker_base_type::get_current_worker().get_num_threads();
}

extern "C"
void __kmpc_barrier(ident* /*id*/, kmp_int32 /*global_tid*/) {
    worker_base_type::get_current_worker().barrier();
}

extern "C"
kmp_int32 __kmpc_ok_to_fork(ident* /*id*/) {
    // Always returns TRUE as the official documentation says.
    return 1;
}

extern "C"
void __kmpc_serialized_parallel(ident* /*id*/, kmp_int32 /*global_tid*/) {
    // Do nothing.
}
extern "C"
void __kmpc_end_serialized_parallel(ident* /*id*/, kmp_int32 /*global_tid*/) {
    // Do nothing.
}


extern "C"
kmp_int32 __kmpc_master(ident* /*loc*/, kmp_int32 /*global_tid*/) {
    return worker_base_type::get_current_worker().get_thread_num() == 0;
}
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
) {
    kmpc_for_static_init_4(loc, gtid, schedtype, plastiter, plower, pupper, pstride, incr, chunk);
}

extern "C"
void __kmpc_for_static_fini(ident* /*loc*/, kmp_int32 /*global_tid*/) {
    // Do nothing.
}


#endif

extern "C" {

extern void* _dsm_data_begin;
extern void* _dsm_data_end;

} // extern "C"

//#define MEOMP_USE_SAME_COMMUNICATOR

int main(int argc, char* argv[])
{
    using namespace menps;
    
    mefdn::disable_aslr(argc, argv);
    
    auto mi =
        mefdn::make_unique<medev2::mpi::direct_requester>(&argc, &argv);
    
    g_argc = argc;
    g_argv = argv;
    
    #ifdef MEOMP_USE_SAME_COMMUNICATOR
    /*const*/ auto win =
        mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    mi->win_lock_all({ 0, win });
    
    auto rma = mecom2::make_mpi_rma(*mi, win);
    auto coll = mecom2::make_mpi_coll(*mi, MPI_COMM_WORLD);
    auto p2p = mecom2::make_mpi_p2p(*mi, MPI_COMM_WORLD);
    
    #else
    const auto coll_comm = mi->comm_dup(MPI_COMM_WORLD);
    const auto rma_comm = mi->comm_dup(MPI_COMM_WORLD);
    const auto p2p_comm = mi->comm_dup(MPI_COMM_WORLD);
    
    /*const*/ auto win =
        mi->win_create_dynamic({ MPI_INFO_NULL, rma_comm });
    
    mi->win_lock_all({ 0, win });
    
    auto rma = mecom2::make_mpi_rma(*mi, win);
    auto coll = mecom2::make_mpi_coll(*mi, coll_comm);
    auto p2p = mecom2::make_mpi_p2p(*mi, p2p_comm);
    #endif
    
    g_coll = &coll;
    
    mefdn::logger::set_state_callback(get_state{});
    
    medsm2::mpi_svm_space sp(*rma, coll, p2p);
    
    g_sp = &sp;
    
    {
        const auto data_begin = reinterpret_cast<mefdn::byte*>(&_dsm_data_begin);
        const auto data_end   = reinterpret_cast<mefdn::byte*>(&_dsm_data_end);
        
        const auto data_size = data_end - data_begin;
        
        if (data_size > 0) {
            mefdn::unique_ptr<mefdn::byte []> init_temp(new mefdn::byte[data_size]);
            
            MEFDN_LOG_VERBOSE(
                "msg:Start saving global variables in temporary buffer.\t"
                "init_temp:0x{:x}\t"
                "data_begin:0x{:x}\t"
                "data_end:0x{:x}\t"
                "data_size:0x{:x}\t"
            ,   reinterpret_cast<mefdn::uintptr_t>(init_temp.get())
            ,   reinterpret_cast<mefdn::uintptr_t>(data_begin)
            ,   reinterpret_cast<mefdn::uintptr_t>(data_end)
            ,   data_size
            );
            
            // Copy the initial data to a buffer (before mmap()).
            #if 1
            // TODO: There is a performance problem of using memcpy()
            //       on a program with many global variables.
            for (mefdn::size_t i = 0; i < data_size; ++i) {
                init_temp[i] = data_begin[i];
            }
            #else
            memcpy(init_temp.get(), data_begin, data_size);
            #endif
            
            sp.coll_alloc_global_var_seg(data_size, 4096 /*TODO*/, data_begin);
            
            if (g_coll->this_proc_id() == 0) {
                sp.enable_on_this_thread();
                
                // Restore the initial data to the global buffer.
                memcpy(data_begin, init_temp.get(), data_size);
            
                sp.disable_on_this_thread();
            }
        }
    }
    
    const auto num_procs = coll.get_num_procs();
    
    const mefdn::size_t stack_size = 16<<10; // TODO
    
    const auto stack_ptr_start =
        sp.coll_alloc_seg((num_procs*meomp::my_dist_worker::get_threads_per_proc()+1) * stack_size, stack_size);
    
    g_stack_ptr = stack_ptr_start;
    g_stack_size = stack_size;
    
    meomp::my_dist_worker dw;
    g_dw = &dw;
    
    dw.loop();
    
    // Do a barrier before exiting.
    g_coll->barrier();
    
    #ifdef MEDSM2_ENABLE_PROF
    for (coll_t::proc_id_type proc = 0; proc < num_procs; ++proc) {
        if (coll.this_proc_id() == proc) {
            fmt::print("- proc: {}\n", proc);
            std::cout << medsm2::prof::to_string("    - ");
        }
        g_coll->barrier();
    }
    #endif
    
    return 0;
}

