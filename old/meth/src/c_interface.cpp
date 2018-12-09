
#include <menps/meth.hpp>
#include <menps/meth.h>
#include "dist_scheduler.hpp"

// DSM

extern "C" {

void* meth_aligned_alloc(const mefdn_size_t alignment, const mefdn_size_t size)
{
    return menps::meth::dsm::untyped::allocate(alignment, size);
}

void* meth_malloc(const mefdn_size_t size)
{
    return meth_aligned_alloc(16 /*TODO*/, size);
}

void meth_free(void* const ptr)
{
    menps::meth::dsm::untyped::deallocate(ptr);
}

} // extern "C"

// ULT

namespace menps {
namespace meth {

extern dist_scheduler_ptr g_sched;

} // namespace meth
} // namespace menps

extern "C" {

meth_new_ult meth_alloc_ult(const mefdn_size_t alignment, const mefdn_size_t size)
{
    auto r = menps::meth::g_sched->allocate(alignment, size);
    
    return { { r.id.ptr }, r.ptr };
}

void meth_fork(const meth_new_ult th, const meth_fork_func_t func)
{
    menps::meth::g_sched->fork(
        { { th.id.p }, th.ptr }
    ,   func
    );
}

void meth_join(const meth_ult_id id) {
    menps::meth::g_sched->join({ id.p });
}

void meth_detach(const meth_ult_id id) {
    menps::meth::g_sched->detach({ id.p });
}

void meth_yield() {
    menps::meth::g_sched->yield();
}

MEFDN_NORETURN
void meth_exit() {
    menps::meth::g_sched->exit();
    MEFDN_UNREACHABLE();
}

} // extern "C"

