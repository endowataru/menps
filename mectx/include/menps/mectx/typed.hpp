
#pragma once

#include <menps/mectx/common.hpp>
#include <menps/mefdn/nontype.hpp>

namespace menps {
namespace mectx {

// make_context

template <typename T, void (*Func)(transfer<T*>)>
inline context<T*> make_context(
    void* const             sp
,   const mefdn::size_t    size
,   mefdn::nontype<
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
,   const mefdn::size_t    size
,   mefdn::nontype<
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
,   mefdn::nontype<
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
MEFDN_NORETURN
inline void restore_context(
    const context<T*>        ctx
,   mefdn::nontype<
        transfer<T*> (*)(Arg*)
    ,   Func
    >                       /*func*/
,   Arg* const              arg
)
{
    restore_context<T, Arg, Func>(ctx, arg);
}

} // namespace mectx
} // namespace menps

