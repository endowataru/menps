
#pragma once

#include <mgth/common.hpp>

namespace mgth {

namespace sched {

typedef void*   thread_id_t;

typedef void (*fork_func_t)(void*);

struct allocated_ult {
    thread_id_t id;
    void*       ptr;
};

allocated_ult allocate_thread(mgbase::size_t alignment, mgbase::size_t size);

void fork(allocated_ult th, fork_func_t func);

void join(thread_id_t id);

void detach(thread_id_t id);

void yield();

MGBASE_NORETURN
void exit();

} // namespace sched

} // namespace mgth

extern "C" {

int mgth_main(int argc, char* argv[]);

} // extern "C"

