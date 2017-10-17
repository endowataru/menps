
#include "wcscasecmp.h"
#include <wctype.h>

#ifdef __cplusplus
extern "C" {
#endif

int wcscasecmp(const wchar_t* lhs, const wchar_t* rhs)
#ifdef __cplusplus
throw()
#endif
{
    wint_t left, right;
    do {
        left = towlower(*lhs++);
        right = towlower(*rhs++);
    }
    while (left && left == right);
    
    return left == right;
}

#ifdef __cplusplus
} // extern "C"
#endif

