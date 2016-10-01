
#include <mgult/basic_worker.hpp>
#include <mgult/basic_scheduler.hpp>
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

#include "ult_desc_pool.hpp"

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
        return desc_pool_.allocate_ult();
    }
    
    void deallocate_ult(ult_ptr_ref&& th)
    {
        desc_pool_.deallocate_ult(mgbase::move(th));
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
    
    ult_desc_pool desc_pool_;
};

__thread my_worker* my_worker::current_worker_ = MGBASE_NULLPTR;


class my_scheduler;

struct my_scheduler_traits
{
    typedef my_scheduler    derived_type;
    typedef my_worker       worker_type;
    typedef worker_rank_t   worker_rank_type;
};

class my_scheduler
    : public basic_scheduler<my_scheduler_traits>
{
    typedef basic_scheduler<my_scheduler_traits>    base;
    
public:
    my_scheduler()
    {
        instance_ = this;
        
        num_ranks_ = get_num_ranks_from_env();
    }
    
    virtual void loop(const loop_func_t func) MGBASE_OVERRIDE
    {
        base::loop(num_ranks_, func);
    }
    
    void set_started()
    {
        finished_.store(false, mgbase::memory_order_relaxed);
    }
    void set_finished()
    {
        finished_.store(true, mgbase::memory_order_release);
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
        MGBASE_ASSERT(stolen_rank < num_ranks);
        
        auto& stolen_wk = this->get_worker_of_rank(stolen_rank);
        
        return stolen_wk.try_steal();
    }
    
    static my_worker& get_current_worker() {
        return my_worker::get_current_worker();
    }
    
    static my_scheduler& get_current_scheduler() {
        return *instance_;
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

