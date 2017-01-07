
#pragma once

#include <mgctx/common.hpp>
#include <mgbase/nontype.hpp>

namespace mgctx {

// make_context

template <typename T, void (*Func)(transfer<T*>)>
inline context<T*> make_context(
    void* const             sp
,   const mgbase::size_t    size
,   mgbase::nontype<
        void (*)(transfer<T*>)
    ,   Func
    >                       /*func*/
) {
    return make_context<T, Func>(sp, size);
}

// save_context

template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
inline transfer<T*> save_context(
    void* const             sp
,   const mgbase::size_t    size
,   mgbase::nontype<
        transfer<T*> (*)(context<T*>, Arg*)
    ,   Func
    >                       /*func*/
,   Arg* const              arg
)
{
    return save_context<T, Arg, Func>(sp, size, arg);
}

// swap_context

template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
inline transfer<T*> swap_context(
    const context<T*>       ctx
,   mgbase::nontype<
        transfer<T*> (*)(context<T*>, Arg*)
    ,   Func
    >                       /*func*/
,   Arg* const              arg
)
{
    return swap_context<T, Arg, Func>(ctx, arg);
}

// restore_context

template <typename T, typename Arg, transfer<T*> (*Func)(Arg*)>
MGBASE_NORETURN
inline transfer<T*> restore_context(
    const context<T*>        ctx
,   mgbase::nontype<
        transfer<T*> (*)(Arg*)
    ,   Func
    >                       /*func*/
,   Arg* const              arg
)
{
    restore_context<T, Arg, Func>(ctx, arg);
}

} // namespace mgctx

