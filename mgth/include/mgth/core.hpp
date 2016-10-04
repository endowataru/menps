
#pragma once

#include <mgbase/lang.hpp>

namespace mgth {

void* allocate(mgbase::size_t size);

void deallocate(void*);


typedef void*   thread_id_t;

typedef void* (*fork_func_t)(void*);

thread_id_t fork(fork_func_t func, void* arg);

void* join(thread_id_t);

void detach(thread_id_t);

void yield();

MGBASE_NORETURN
void exit(void* ret);

} // namespace mgth


int mgth_main(int argc, char* argv[]);

