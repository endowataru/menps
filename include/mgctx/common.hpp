
#pragma once

#include <mgbase/lang.hpp>

namespace mgctx {

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
,   mgbase::size_t  size
);

template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
inline transfer<T*> save_context(
    void*           sp
,   mgbase::size_t  size
,   Arg*            arg
);

template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
inline transfer<T*> swap_context(
    context<T*>     ctx
,   Arg*            arg
);

template <typename T, typename Arg, transfer<T*> (*Func)(Arg*)>
MGBASE_NORETURN
inline void restore_context(
    context<T*>     ctx
,   Arg*            arg
);

} // namespace mgctx

