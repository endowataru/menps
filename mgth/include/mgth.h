
#pragma once

#include <mgbase/lang.h>

MGBASE_EXTERN_C_BEGIN

// DSM

void * mgth_aligned_alloc(mgbase_size_t alignment, mgbase_size_t size);

void * mgth_malloc(mgbase_size_t size);

void mgth_free(void * ptr);

// ULT

typedef struct mgth_ult_id
{
    void *      p;
}
mgth_ult_id;

typedef struct mgth_new_ult
{
    mgth_ult_id id;
    void *      ptr;
}
mgth_new_ult;

typedef void (* mgth_fork_func_t)(void*);

mgth_new_ult mgth_alloc_ult(mgbase_size_t alignment, mgbase_size_t size);

void mgth_fork(mgth_new_ult th, mgth_fork_func_t func);

void mgth_join(mgth_ult_id id);

void mgth_detach(mgth_ult_id id);

void mgth_yield();

MGBASE_NORETURN
void mgth_exit();

int mgth_main(int argc, char * argv[]);

MGBASE_EXTERN_C_END

