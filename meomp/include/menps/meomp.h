
#pragma once

#include <stddef.h>

#if defined(__APPLE__)
    // FIXME: There's no good way to implement global variables in macOS.
    #define MEOMP_GLOBAL_VAR
#else
    #define MEOMP_GLOBAL_VAR    __attribute__((section(".dsm_data")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int meomp_get_num_procs(void);

extern int meomp_get_proc_num(void);

extern int meomp_get_local_thread_num(void);

extern int meomp_get_num_local_threads(void);

extern void* meomp_malloc(size_t);

extern void meomp_free(void*);

extern void meomp_local_barrier(void);

extern void meomp_prof_begin(void);

extern void meomp_prof_end(void);

#ifdef __cplusplus
} // extern "C"
#endif

