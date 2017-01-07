
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

namespace untyped {

typedef context<void*>  context_t;

typedef transfer<void*> transfer_t;

template <void (*Func)(transfer_t)>
inline context_t make_context(
    void*           sp
,   mgbase::size_t  size
);

template <transfer_t (*Func)(context_t, void*)>
inline transfer_t save_context(
    void*           sp
,   mgbase::size_t  size
,   void*           arg
);

template <transfer_t (*Func)(context_t, void*)>
MGBASE_NORETURN
inline transfer_t swap_context(
    context_t   ctx
,   void*       arg
);

template <transfer_t (*Func)(void*)>
MGBASE_NORETURN
inline void restore_context(
    context_t   ctx
,   void*       arg
);

} // namespace untyped

} // namespace mgctx

