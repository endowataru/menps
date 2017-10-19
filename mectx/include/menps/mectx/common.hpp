
#pragma once

#include <menps/mefdn/lang.hpp>

#define MECTX_SWITCH_FUNCTION   MEFDN_VISIBILITY_HIDDEN

#ifndef MEFDN_COMPILER_CLANG
    // TODO: It seems that Clang cannot provide template arguments
    //       to the inline assembly parameters as immediate values.
    //       The performance is degraded by disabling this flag.
    #define MECTX_AVOID_PLT
#endif

namespace menps {
namespace mectx {

struct context_frame;

template <typename T>
struct context;

template <typename T>
struct context<T*>
{
    context_frame*  p;
};

template <typename T>
struct transfer;

template <typename T>
struct transfer<T*>
{
    T*  p0;
};

template <typename T, void (*Func)(transfer<T*>)>
inline context<T*> make_context(
    void*           sp
,   mefdn::size_t  size
);

template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
inline transfer<T*> save_context(
    void*           sp
,   mefdn::size_t  size
,   Arg*            arg
);

template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
inline transfer<T*> swap_context(
    context<T*>     ctx
,   Arg*            arg
);

template <typename T, typename Arg, transfer<T*> (*Func)(Arg*)>
MEFDN_NORETURN
inline void restore_context(
    context<T*>     ctx
,   Arg*            arg
);

} // namespace mectx
} // namespace menps

