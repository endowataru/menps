
#include <mgth.hpp>
//#include <mgas2.hpp>
#include <mgdsm.hpp>
#include "dist_scheduler.hpp"

#include "disable_aslr.hpp"

#include <mgbase/external/malloc.h>

namespace mgth {

namespace /*unnamed*/ {

mgdsm::space_ref g_dsm;
mgdsm::segment_ref g_seg_heap;

class contiguous_allocator
    : public mgbase::allocatable
{
public:
    contiguous_allocator(void* const ptr, const mgbase::size_t size)
        : ms_(::create_mspace_with_base(ptr, size, 1))
    { }
    
    contiguous_allocator()
    {
        ::destroy_mspace(this->ms_);
    }
    
    virtual void* aligned_alloc(
        const mgbase::size_t //alignment // TODO
    ,   const mgbase::size_t size_in_bytes
    )
    MGBASE_OVERRIDE
    {
        return ::mspace_malloc(this->ms_, size_in_bytes);
    }
    
    virtual void free(void* const ptr) MGBASE_OVERRIDE
    {
        ::mspace_free(this->ms_, ptr);
    }
    
private:
    ::mspace ms_;
};

mgbase::unique_ptr<contiguous_allocator> g_heap_alloc;

dist_scheduler_ptr g_sched;

int g_argc;
char** g_argv;
int g_ret;

void main_handler()
{
    // Call the user-defined main function.
    int ret = mgth_main(g_argc, g_argv);
}

inline sched::thread_id_t to_thread_id(const mgult::ult_id& id) {
    return reinterpret_cast<sched::thread_id_t>(id.ptr);
}
inline mgult::ult_id to_ult_id(const sched::thread_id_t& id) {
    return { reinterpret_cast<void*>(id) };
}

} // unnamed namespace

namespace sched {

allocated_ult allocate_thread(const mgbase::size_t alignment, const mgbase::size_t size)
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

MGBASE_NORETURN
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

void* allocate(const mgbase::size_t alignment, const mgbase::size_t size_in_bytes)
{
    return g_heap_alloc->aligned_alloc(alignment, size_in_bytes);
}

void deallocate(void* const p)
{
    g_heap_alloc->free(p);
}

} // namespace untyped

} // namespace dsm

} // namespace mgth


int main(int argc, char* argv[])
{
    mgth::disable_aslr(argc, argv);
    
    // The communication interface is initialized here. (e.g. MPI_Init() is called.)
    mgcom::initialize(&argc, &argv);
    
    mgth::g_argc = argc;
    mgth::g_argv = argv;
    
    // Initialize DSM.
    mgth::g_dsm = mgdsm::make_space();
    
    {
        auto stack_seg = mgth::g_dsm.make_segment(64ull << 20, 16ull << 10, 4096 /*4KB*/);
        
        // Enable SEGV handler temporarily to allocate the heap.
        mgth::g_dsm.enable_on_this_thread();
        
        mgth::g_seg_heap = mgth::g_dsm.make_segment(64ull << 20, 16ull << 10, 4096 /*4KB*/);
        mgth::g_heap_alloc = mgbase::make_unique<mgth::contiguous_allocator>(mgth::g_seg_heap.get_ptr(), mgth::g_seg_heap.get_size_in_bytes());
        
        // Disable SEGV handler again.
        mgth::g_dsm.disable_on_this_thread();
        
        //mgth::dsm::segment_allocator stack_alloc(stack_seg.get_ptr(), stack_seg.get_size_in_bytes());
        
        // Initialize a scheduler.
        mgth::g_sched = mgth::make_dist_scheduler({ mgth::g_dsm, stack_seg.get_ptr() });
        
        // Start the scheduler loop.
        if (mgcom::current_process_id() == 0) {
            // Add the main thread.
            mgth::g_sched->loop(&mgth::main_handler);
        }
        else {
            // Start as normal workers.
            mgth::g_sched->loop(mgth::loop_func_type{});
        }
        
        // Finalize the scheduler.
        mgth::g_sched.reset();
        
    }
    
    // Finalize DSM.
    mgth::g_dsm = mgdsm::space_ref{};
    
    // Finalize the communication interface.
    mgcom::finalize();
    
    return mgth::g_ret;
}

