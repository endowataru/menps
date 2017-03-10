
#include <mgth.hpp>
#include <mgth.h>
#include "dist_scheduler.hpp"

// DSM

extern "C" {

void* mgth_aligned_alloc(const mgbase_size_t alignment, const mgbase_size_t size)
{
    return mgth::dsm::untyped::allocate(alignment, size);
}

void* mgth_malloc(const mgbase_size_t size)
{
    return mgth_aligned_alloc(16 /*TODO*/, size);
}

void mgth_free(void* const ptr)
{
    mgth::dsm::untyped::deallocate(ptr);
}

} // extern "C"

// ULT

namespace mgth {

extern dist_scheduler_ptr g_sched;

} // namespace mgth

extern "C" {

mgth_new_ult mgth_alloc_ult(const mgbase_size_t alignment, const mgbase_size_t size)
{
    auto r = mgth::g_sched->allocate(alignment, size);
    
    return { { r.id.ptr }, r.ptr };
}

void mgth_fork(const mgth_new_ult th, const mgth_fork_func_t func)
{
    mgth::g_sched->fork(
        { { th.id.p }, th.ptr }
    ,   func
    );
}

void mgth_join(const mgth_ult_id id) {
    mgth::g_sched->join({ id.p });
}

void mgth_detach(const mgth_ult_id id) {
    mgth::g_sched->detach({ id.p });
}

void mgth_yield() {
    mgth::g_sched->yield();
}

MGBASE_NORETURN
void mgth_exit() {
    mgth::g_sched->exit();
    MGBASE_UNREACHABLE();
}

} // extern "C"

