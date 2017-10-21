
#include <menps/meth.hpp>
#include <menps/medsm.hpp>
#include <menps/mecom.hpp>
#include "dist_scheduler.hpp"

#include "disable_aslr.hpp"

#include <menps/mefdn/external/malloc.h>
#include <menps/mefdn/arithmetic.hpp>
#include <menps/mefdn/memory/next_in_bytes.hpp>

namespace menps {
namespace meth {

void initialize_misc();

namespace /*unnamed*/ {

medsm::space_ref g_dsm;
medsm::segment_ref g_seg_heap;

#if 0
class contiguous_allocator
    : public mefdn::allocatable
{
public:
    contiguous_allocator(void* const ptr, const mefdn::size_t size)
        : ms_(::create_mspace_with_base(ptr, size, 1))
    { }
    
    contiguous_allocator()
    {
        ::destroy_mspace(this->ms_);
    }
    
    virtual void* aligned_alloc(
        const mefdn::size_t //alignment // TODO
    ,   const mefdn::size_t size_in_bytes
    )
    MEFDN_OVERRIDE
    {
        return ::mspace_malloc(this->ms_, size_in_bytes);
    }
    
    virtual void free(void* const ptr) MEFDN_OVERRIDE
    {
        ::mspace_free(this->ms_, ptr);
    }
    
private:
    ::mspace ms_;
};

mefdn::unique_ptr<contiguous_allocator> g_heap_alloc;
#endif

class count_allocator
    : public mefdn::allocatable
{
public:
    count_allocator(void* const ptr, const mefdn::size_t size)
        : ptr_(ptr)
        , current_(0)
        , size_(size)
    { }
    
    virtual void* aligned_alloc(
        const mefdn::size_t alignment // TODO
    ,   const mefdn::size_t size_in_bytes
    )
    MEFDN_OVERRIDE
    {
        MEFDN_LOG_VERBOSE(
            "msg:Allocate DSM heap.\t"
            "alignment:{}\t"
            "requested:{}\t"
            "ptr:0x{:x}\t"
            "current:{}\t"
            "size:{}"
        ,   alignment
        ,   size_in_bytes
        ,   reinterpret_cast<mefdn::uintptr_t>(ptr_)
        ,   current_
        ,   size_
        );
        
        MEFDN_ASSERT(alignment > 0);
        
        const auto allocated_size =
            mefdn::roundup_divide(size_in_bytes, alignment) * alignment;
        
        MEFDN_ASSERT(this->current_ + size_in_bytes < this->size_);
        
        const auto ret = mefdn::next_in_bytes(ptr_, this->current_);
        this->current_ += size_in_bytes;
        
        return ret;
    }
    
    virtual void free(void* const ptr) MEFDN_OVERRIDE
    {
        MEFDN_LOG_VERBOSE("msg:Free is not implemented.");
    }
    
private:
    void* ptr_;
    mefdn::size_t current_;
    mefdn::size_t size_;
};

mefdn::unique_ptr<count_allocator> g_heap_alloc;

int g_argc;
char** g_argv;
int g_ret;

int (*g_main)(int, char**);

void main_handler()
{
    meth::g_dsm.read_barrier();
    
    // Call the user-defined main function.
    g_ret = g_main(g_argc, g_argv);
}

inline sched::thread_id_t to_thread_id(const meult::ult_id& id) {
    return reinterpret_cast<sched::thread_id_t>(id.ptr);
}
inline meult::ult_id to_ult_id(const sched::thread_id_t& id) {
    return { reinterpret_cast<void*>(id) };
}

} // unnamed namespace

dist_scheduler_ptr g_sched;

namespace sched {

allocated_ult allocate_thread(const mefdn::size_t alignment, const mefdn::size_t size)
{
    auto r = g_sched->allocate(alignment, size);
    
    return { to_thread_id(r.id), r.ptr };
}

void fork(const allocated_ult th, fork_func_t func)
{
    g_sched->fork({ to_ult_id(th.id), th.ptr }, func);
}

void join(const thread_id_t id) {
    return g_sched->join(to_ult_id(id));
}

void detach(const thread_id_t id) {
    return g_sched->detach(to_ult_id(id));
}

void yield() {
    g_sched->yield();
}

MEFDN_NORETURN
void exit() {
    g_sched->exit();
}

} // namespace sched

namespace ult {

scheduler& get_scheduler()
{
    return *g_sched;
}

} // namespace ult

namespace dsm {

namespace untyped {

void* allocate(const mefdn::size_t alignment, const mefdn::size_t size_in_bytes)
{
    return g_heap_alloc->aligned_alloc(alignment, size_in_bytes);
}

void deallocate(void* const p)
{
    g_heap_alloc->free(p);
}

} // namespace untyped

} // namespace dsm

namespace /*unnamed*/ {

char** copy_argv_to_dsm(int argc, char** argv)
{
    const auto ret =
        static_cast<char**>(
            dsm::untyped::allocate(alignof(char*), argc * sizeof(char*))
        );
    
    for (int i = 0; i < argc; ++i) {
        const mefdn::size_t len = strlen(argv[i]);
        const auto arg =
            static_cast<char*>(
                dsm::untyped::allocate(alignof(char), (len+1) * sizeof(char))
            );
        
        strcpy(arg, argv[i]);
        
        ret[i] = arg;
    }
    
    return ret;
}

} // unnamed namespace

int start(int argc, char* argv[], int (*f)(int, char**))
{
    meth::disable_aslr(argc, argv);
    
    // The communication interface is initialized here. (e.g. MPI_Init() is called.)
    mecom::initialize(&argc, &argv);
    
    meth::initialize_misc();
    
    // Initialize DSM.
    meth::g_dsm = medsm::make_space();
    
    {
        auto stack_seg = meth::g_dsm.make_segment(64ull << 20, 16ull << 10, 4096 /*4KB*/);
        
        // Enable SEGV handler temporarily to allocate the heap.
        //meth::g_dsm.enable_on_this_thread();
        
        meth::g_seg_heap = meth::g_dsm.make_segment(0x300000000000 / 1024, 16ull << 10, 4096 /*4KB*/);
        //meth::g_heap_alloc = mefdn::make_unique<meth::contiguous_allocator>(meth::g_seg_heap.get_ptr(), meth::g_seg_heap.get_size_in_bytes());
        meth::g_heap_alloc = mefdn::make_unique<meth::count_allocator>(meth::g_seg_heap.get_ptr(), meth::g_seg_heap.get_size_in_bytes());
        
        // Disable SEGV handler again.
        //meth::g_dsm.disable_on_this_thread();
        
        //meth::dsm::segment_allocator stack_alloc(stack_seg.get_ptr(), stack_seg.get_size_in_bytes());
        
        // Initialize a scheduler.
        meth::g_sched = meth::make_dist_scheduler({ meth::g_dsm, stack_seg.get_ptr() });
        
        // Set up the start-up variables.
        // TODO: avoid using global vars
        g_main = f;
        meth::g_argc = argc;
        
        // Set up argv.
        if (mecom::current_process_id() == 0) {
            meth::g_dsm.enable_on_this_thread();
            meth::g_argv = copy_argv_to_dsm(argc, argv);
            meth::g_dsm.write_barrier();
            meth::g_dsm.disable_on_this_thread();
        }
        
        // Broadcast "argv" (because the start-up thread may be executed in other processes.)
        mecom::collective::broadcast(0, &meth::g_argv, 1);
        
        // Start the scheduler loop.
        if (mecom::current_process_id() == 0) {
            // Add the main thread.
            meth::g_sched->loop(&meth::main_handler);
        }
        else {
            // Start as normal workers.
            meth::g_sched->loop(meth::loop_func_type{});
        }
        
        // Finalize the scheduler.
        meth::g_sched.reset();
        
    }
    
    // Finalize DSM.
    meth::g_dsm = medsm::space_ref{};
    
    // Finalize the communication interface.
    mecom::finalize();
    
    return meth::g_ret;
}

} // namespace meth
} // namespace menps

