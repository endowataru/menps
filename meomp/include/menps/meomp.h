
#pragma once

#include <stddef.h>

#define MEOMP_GLOBAL_VAR    __attribute__((section(".dsm_data")))

#ifdef __cplusplus
extern "C" {
#endif

extern int meomp_get_num_procs(void);

extern void* meomp_malloc(size_t);

extern void meomp_free(void*);

#ifdef __cplusplus
} // extern "C"
#endif

