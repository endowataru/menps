
#pragma once

#include <mgctx/common.hpp>
#include <mgbase/nontype.hpp>

namespace mgctx {

// make_context

namespace detail {

template <typename T, void (*Func)(transfer<T>)>
inline void on_start(void* const data)
{
    Func({ static_cast<T*>(data) });
}

} // namespace detail

template <typename T, void (*Func)(transfer<T>)>
inline context<T> make_context(
    void* const             sp
,   const mgbase::size_t    size
) {
    return untyped::make_context<&detail::on_start<T, Func>>(
        sp
    ,   size
    );
}


// save_context

namespace detail {

template <typename T, typename Arg, transfer<T> (*Func)(context<T>, Arg*)>
inline untyped::transfer_t on_saved(
    const untyped::context_t    ctx
,   void* const                 arg
)
{
    const auto r =
        Func({ ctx.p }, static_cast<Arg*>(arg));
    
    return { r.p0 };
}

} // namespace detail

template <typename T, typename Arg, transfer<T> (*Func)(context<T>, Arg*)>
inline transfer<T> save_context(
    void* const             sp
,   const mgbase::size_t    size
,   Arg* const              arg
)
{
    const auto r =
        untyped::save_context<&detail::on_saved<T, Arg, Func>>(
            sp
        ,   size
        ,   arg
        );
    
    return { static_cast<T>(r.p0) };
}
template <typename T, typename Arg, transfer<T> (*Func)(context<T>, Arg*)>
inline transfer<T> save_context(
    void* const             sp
,   const mgbase::size_t    size
,   mgbase::nontype<
        transfer<T> (*)(context<T>, Arg*)
    ,   Func
    >                       /*func*/
,   Arg* const              arg
)
{
    return save_context<T, Arg, Func>(sp, size, arg);
}


// restore_context

template <typename T>
MGBASE_NORETURN
inline void restore_context(const context<T> ctx, const transfer<T> tr)
{
    untyped::restore_context({ ctx.p }, { tr.p0 });
}

} // namespace mgctx

