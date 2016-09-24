
#include <mgult/basic_worker.hpp>
#include "ult_ptr_ref.hpp"
#include <mgult/locked_worker_deque.hpp>
#include <mgult/default_worker_deque.hpp>
#include "ptr_worker_deque.hpp"

#include <mgult/scheduler.hpp>

#include <mgbase/unique_ptr.hpp>
#include <mgbase/shared_ptr.hpp>

#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/threading/thread.hpp>

#include <mgbase/profiling/stopwatch.hpp>

#include <vector>

#define MGULT_PROFILE_WORKER

namespace mgult {

//namespace /*unnamed*/ {

typedef mgbase::size_t      worker_rank_t;

class my_worker;

struct my_worker_traits
{
    typedef my_worker       derived_type;
    typedef ult_ptr_ref     ult_ref_type;
    
    //typedef locked_worker_deque<ult_ref_type>   worker_deque_type;
    typedef ptr_worker_deque<default_worker_deque>  worker_deque_type;
    
    template <typename B, typename A>
    struct context {
        typedef fcontext<B, A>  type;
    };
    template <typename B, typename A>
    struct context_result {
        typedef fcontext_result<B, A>   type;
    };
    template <typename B, typename A>
    struct context_argument {
        typedef fcontext_argument<B, A> type;
    };
};

class my_scheduler;

class my_worker
    : public basic_worker<my_worker_traits>
{
public:
    my_worker(my_scheduler& sched, const worker_rank_t rank)
        : sched_(sched)
        , rank_(rank)
        { }
    
    ult_ptr_ref allocate_ult()
    {
        auto desc = new ult_desc;
        
        #ifdef MGULT_PROFILE_WORKER
        mgbase::stopwatch sw;
        sw.start();
        #endif
        
        desc->state = ult_state::ready;
        desc->detached = false;
        
        const auto stack_size = 2048 * 1024; // TODO
        
        const auto sp_start = new mgbase::uint8_t[stack_size];
        // Set the end of the call stack.
        desc->stack_ptr = sp_start + stack_size;
        desc->stack_size = stack_size;
        
        desc->joiner = static_cast<ult_desc*>(make_invalid_ult_id().ptr);
        
        MGBASE_LOG_VERBOSE(
            "msg:Allocate a new thread descriptor.\t"
            "desc:{:x}\t"
            "stack_ptr:{:x}\t"
            "stack_size:{}"
        ,   reinterpret_cast<mgbase::uintptr_t>(desc)
        ,   reinterpret_cast<mgbase::uintptr_t>(desc->stack_ptr)
        ,   desc->stack_size
        );
        
        #ifdef MGULT_PROFILE_WORKER
        alloc_cycles_ += sw.elapsed();
        #endif
        
        ult_id id{ desc };
        return ult_ptr_ref(id);
    }
    
    void deallocate_ult(ult_ptr_ref&& th)
    {
        #ifdef MGULT_PROFILE_WORKER
        mgbase::stopwatch sw;
        sw.start();
        #endif
        
        const auto id = th.get_id();
        
        auto desc = static_cast<ult_desc*>(id.ptr);
        
        const auto sp_end = static_cast<mgbase::uint8_t*>(desc->stack_ptr);
        const auto sp_start = sp_end - desc->stack_size;
        
        MGBASE_LOG_VERBOSE(
            "msg:Deallocate the thread descriptor.\t"
            "desc:{:x}\t"
            "stack_ptr:{:x}\t"
            "stack_size:{}\t"
            "sp_start:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(desc)
        ,   reinterpret_cast<mgbase::uintptr_t>(desc->stack_ptr)
        ,   desc->stack_size
        ,   reinterpret_cast<mgbase::uintptr_t>(sp_start)
        );
        
        delete[] sp_start;
        
        delete desc;
        
        #ifdef MGULT_PROFILE_WORKER
        dealloc_cycles_ += sw.elapsed();
        #endif
    }
    
    ult_ptr_ref get_ult_ref_from_id(const ult_id& id)
    {
        return ult_ptr_ref(id);
    }
    
    template <typename B2, typename A2, typename B1, typename A1>
    static fcontext<B2, A2> cast_context(
        const fcontext<B1, A1>  fctx
    ) {
        // Ignore types.
        return { fctx.fctx };
    }
    
    template <typename C, typename B, typename A>
    static fcontext_result<C, B> jump_context(
        const fcontext<B, A>    fctx
    ,   A* const                data
    ) {
        return jump_fcontext<C>(fctx, data);
    }
    
    template <typename C, typename B, typename A, typename T>
    static fcontext_result<C, A> ontop_context(
        const fcontext<B, A>    fctx
    ,   T* const                data
    ,   fcontext_result<C, A>   (* const func)(fcontext_argument<B, T>)
    ) {
        return ontop_fcontext(fctx, data, func);
    }
    
    template <typename C, typename B, typename A>
    static fcontext_result<C, B> jump_new_context(
        void* const             stack_ptr
    ,   const mgbase::size_t    stack_size
    ,   void                    (* const func)(fcontext_argument<B, A>)
    ,   A* const                data
    )
    {
        const auto ctx = make_fcontext(stack_ptr, stack_size, func);
        
        return jump_context<C>(ctx, data);
    }
    
    std::string show_ult_ref(ult_ptr_ref& th)
    {
        fmt::MemoryWriter w;
        w.write(
            "rank:{}\t"
            "{}"
        ,   get_rank()
        ,   th.to_string()
        );
        
        return w.str();
    }
    
    worker_rank_t get_rank() const MGBASE_NOEXCEPT {
        return rank_;
    }
    
    void initialize_on_this_thread()
    {
        MGBASE_ASSERT(current_worker_ == MGBASE_NULLPTR);
        
        current_worker_ = this;
        
        MGBASE_LOG_VERBOSE(
            "msg:Starting worker.\t"
            "rank:{}"
        ,   this->get_rank()
        );
    }
    
    void finalize_on_this_thread()
    {
        MGBASE_ASSERT(current_worker_ != MGBASE_NULLPTR);
        
        MGBASE_LOG_VERBOSE(
            "msg:Finishing worker.\t"
            "rank:{}\t"
        ,   this->get_rank()
        );
        
        fmt::print(
            "rank:{}\t"
            "alloc:{}\t"
            "dealloc:{}\n"
        ,   this->get_rank()
        ,   alloc_cycles_
        ,   dealloc_cycles_
        );
        
        current_worker_ = MGBASE_NULLPTR;
    }
    
    static my_worker& get_current_worker() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(current_worker_ != MGBASE_NULLPTR);
        return *current_worker_;
    }
    
    void check_current_worker() {
        MGBASE_ASSERT(this == &get_current_worker());
    }
    
    static my_worker& renew_worker(const ult_id /*id*/) {
        return get_current_worker();
    }
    
    bool finished();
    
    ult_ptr_ref try_steal_from_another();
    
private:
    my_scheduler& sched_;
    worker_rank_t rank_;
    
    static __thread my_worker* current_worker_;
    
    mgbase::cpu_clock_t alloc_cycles_ = 0;
    mgbase::cpu_clock_t dealloc_cycles_ = 0;
};

__thread my_worker* my_worker::current_worker_ = MGBASE_NULLPTR;

class my_scheduler
    : public scheduler
{
public:
    my_scheduler()
    {
        instance_ = this;
        
        num_ranks_ = get_num_ranks_from_env();
    }
    
private:
    struct main_thread_data
    {
        void (*func)();
    };
    
    static void* main_thread_handler(void* const arg)
    {
        const auto f = reinterpret_cast<loop_func_t>(arg);
        
        f();
        
        instance_->finished_.store(true, mgbase::memory_order_release);
        
        return MGBASE_NULLPTR;
    }
    
    struct worker_loop_functor
    {
        my_worker&  wk;
        
        void operator() ()
        {
            wk.initialize_on_this_thread();
            
            wk.loop();
            
            wk.finalize_on_this_thread();
        }
    };
    
    struct worker_loop_main_functor
    {
        my_worker&  wk;
        loop_func_t func;
        
        void operator() ()
        {
            wk.initialize_on_this_thread();
            
            // FIXME: detach
            const auto t = wk.fork_parent_first(&main_thread_handler, reinterpret_cast<void*>(func));
            
            //wk.detach(t);
            
            wk.loop();
            
            wk.finalize_on_this_thread();
        }
    };
    
public:
    virtual void loop(const loop_func_t func) MGBASE_OVERRIDE
    {
        finished_.store(false);
        
        workers_.resize(num_ranks_);
        
        for (worker_rank_t rank = 0; rank < get_num_ranks(); ++rank)
        {
            auto& info = workers_[rank];
            info.wk = mgbase::make_shared<my_worker>(*this, rank);
        }
        
        {
            auto& info0 = workers_[0];
            info0.real_th = mgbase::make_shared<mgbase::thread>(
                mgbase::thread(worker_loop_main_functor{ *info0.wk, func })
            );
        }
        
        for (worker_rank_t rank = 1; rank < get_num_ranks(); ++rank)
        {
            auto& info = workers_[rank];
            info.real_th = mgbase::make_shared<mgbase::thread>(
                mgbase::thread(worker_loop_functor{ *info.wk })
            );
        }
        
        MGBASE_LOG_VERBOSE(
            "msg:Started all workers."
        );
        
        MGBASE_RANGE_BASED_FOR(auto& info, workers_)
        {
            info.real_th->join();
        }
        
        MGBASE_LOG_VERBOSE(
            "msg:All workers finished."
        );
        
        workers_.clear();
    }
    
    virtual ult_id fork(const fork_func_t func, void* const arg) MGBASE_OVERRIDE
    {
        return my_worker::get_current_worker().fork_child_first(func, arg);
    }
    
    virtual void* join(const ult_id& id) MGBASE_OVERRIDE
    {
        return my_worker::get_current_worker().join(id);
    }
    
    virtual void detach(const ult_id& id) MGBASE_OVERRIDE
    {
        return my_worker::get_current_worker().detach(id);
    }
    
    virtual void yield() MGBASE_OVERRIDE
    {
        my_worker::get_current_worker().yield();
    }
    
    MGBASE_NORETURN
    virtual void exit(void* const ret) MGBASE_OVERRIDE
    {
        my_worker::get_current_worker().exit(ret);
    }
    
    bool finished() const MGBASE_NOEXCEPT
    {
        return finished_.load(mgbase::memory_order_acquire);
    }
    
    ult_ptr_ref try_steal_from_another(my_worker& w)
    {
        MGBASE_ASSERT(&my_worker::get_current_worker() == &w);
        
        const auto num_ranks = get_num_ranks();
        
        if (num_ranks == 1)
            return ult_ptr_ref{};
        
        const auto current_rank = w.get_rank();
        
        // TODO: better algorithm
        const auto random_val = static_cast<worker_rank_t>(std::rand());
        
        const auto stolen_rank = (current_rank + random_val % (num_ranks - 1) + 1) % num_ranks;
        
        MGBASE_ASSERT(stolen_rank != current_rank);
        MGBASE_ASSERT(stolen_rank < get_num_ranks());
        
        auto& stolen_info = workers_[stolen_rank];
        
        return stolen_info.wk->try_steal();
    }
    
private:
    worker_rank_t get_num_ranks() const MGBASE_NOEXCEPT {
        return num_ranks_;
    }
    
    static worker_rank_t get_num_ranks_from_env()
    {
        const auto s = getenv("MGULT_NUM_WORKERS");
        if (s != MGBASE_NULLPTR) {
            auto num_ranks = static_cast<worker_rank_t>(atoi(s));
            if (num_ranks == 0) {
                // TODO: Should it be an exception?
                return 1; // Default
            }
            return num_ranks;
        }
        else
            return 1; // Default
    }
    
    mgbase::atomic<bool> finished_;
    
    struct worker_info {
        // TODO: Currently shared_ptr is used
        //       because old libstdc++'s vector doesn't allow move-only types as elements.
        //       They are unique in fact.
        mgbase::shared_ptr<mgbase::thread>  real_th;
        mgbase::shared_ptr<my_worker>       wk;
    };
    
    std::vector<worker_info> workers_;
    
    worker_rank_t num_ranks_;
    
    static my_scheduler* instance_; // FIXME
};


my_scheduler* my_scheduler::instance_; // FIXME

bool my_worker::finished()
{
    return sched_.finished();
}

ult_ptr_ref my_worker::try_steal_from_another()
{
    return sched_.try_steal_from_another(*this);
}

//} // unnamed namespace

mgbase::unique_ptr<scheduler> make_scheduler()
{
    return mgbase::make_unique<my_scheduler>();
}

} // namespace mgult

