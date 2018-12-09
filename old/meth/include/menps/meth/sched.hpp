
#pragma once

#include <menps/meth/common.hpp>

namespace menps {
namespace meth {

namespace sched {

typedef void*   thread_id_t;

typedef void (*fork_func_t)(void*);

struct allocated_ult {
    thread_id_t id;
    void*       ptr;
};

allocated_ult allocate_thread(mefdn::size_t alignment, mefdn::size_t size);

void fork(allocated_ult th, fork_func_t func);

void join(thread_id_t id);

void detach(thread_id_t id);

void yield();

MEFDN_NORETURN
void exit();

} // namespace sched

} // namespace meth
} // namespace menps

extern "C" {

int meth_main(int argc, char* argv[]);

} // extern "C"

