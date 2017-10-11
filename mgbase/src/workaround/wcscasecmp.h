
#pragma once

#include <wchar.h>

// Workaround for Fujitsu compiler

#ifdef __cplusplus
extern "C" {
#endif

int wcscasecmp(const wchar_t* lhs, const wchar_t* rhs)
#ifdef __cplusplus
throw()
#endif
;

#ifdef __cplusplus
} // extern "C"
#endif

