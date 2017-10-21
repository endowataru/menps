
#pragma once

#include <menps/meth/config.h>
#include <menps/mefdn/lang.hpp> // TODO: C++

MEFDN_EXTERN_C_BEGIN

typedef size_t mefdn_size_t;

// DSM

void * meth_aligned_alloc(mefdn_size_t alignment, mefdn_size_t size);

void * meth_malloc(mefdn_size_t size);

void meth_free(void * ptr);

// ULT

typedef struct meth_ult_id
{
    void *      p;
}
meth_ult_id;

typedef struct meth_new_ult
{
    meth_ult_id id;
    void *      ptr;
}
meth_new_ult;

typedef void (* meth_fork_func_t)(void*);

meth_new_ult meth_alloc_ult(mefdn_size_t alignment, mefdn_size_t size);

void meth_fork(meth_new_ult th, meth_fork_func_t func);

void meth_join(meth_ult_id id);

void meth_detach(meth_ult_id id);

void meth_yield();

MEFDN_NORETURN
void meth_exit();

int meth_main(int argc, char * argv[]);

MEFDN_EXTERN_C_END

