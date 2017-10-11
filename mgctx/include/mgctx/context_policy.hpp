
#pragma once

#include <mgctx.hpp>

namespace mgctx {

struct context_policy
{
    template <typename T, void (*Func)(transfer<T*>)>
    static context<T*> make_context(
        void* const             sp
    ,   const mgbase::size_t    size
    ,   mgbase::nontype<
            void (*)(transfer<T*>)
        ,   Func
        >                       /*func*/
    ) {
        return mgctx::make_context<T, Func>(sp, size);
    }
    
    template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
    static transfer<T*> save_context(
        void* const             sp
    ,   const mgbase::size_t    size
    ,   mgbase::nontype<
            transfer<T*> (*)(context<T*>, Arg*)
        ,   Func
        >                       /*func*/
    ,   Arg* const              arg
    )
    {
        return mgctx::save_context<T, Arg, Func>(sp, size, arg);
    }
    
    template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
    static transfer<T*> swap_context(
        const context<T*>       ctx
    ,   mgbase::nontype<
            transfer<T*> (*)(context<T*>, Arg*)
        ,   Func
        >                       /*func*/
    ,   Arg* const              arg
    )
    {
        return mgctx::swap_context<T, Arg, Func>(ctx, arg);
    }

    // restore_context

    template <typename T, typename Arg, transfer<T*> (*Func)(Arg*)>
    MGBASE_NORETURN
    static void restore_context(
        const context<T*>        ctx
    ,   mgbase::nontype<
            transfer<T*> (*)(Arg*)
        ,   Func
        >                       /*func*/
    ,   Arg* const              arg
    )
    {
        mgctx::restore_context<T, Arg, Func>(ctx, arg);
    }
};

} // namespace mgctx

