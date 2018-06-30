
#pragma once

#define MEOMP_GLOBAL_VAR    __attribute__((section(".dsm_data")))

#ifdef __cplusplus
extern "C" {
#endif

extern int meomp_get_num_procs(void);

#ifdef __cplusplus
} // extern "C"
#endif

