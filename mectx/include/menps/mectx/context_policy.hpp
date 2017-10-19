
#pragma once

#include <menps/mectx.hpp>

namespace menps {
namespace mectx {

struct context_policy
{
    template <typename T, void (*Func)(transfer<T*>)>
    static context<T*> make_context(
        void* const             sp
    ,   const mefdn::size_t    size
    ,   mefdn::nontype<
            void (*)(transfer<T*>)
        ,   Func
        >                       /*func*/
    ) {
        return mectx::make_context<T, Func>(sp, size);
    }
    
    template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
    static transfer<T*> save_context(
        void* const             sp
    ,   const mefdn::size_t    size
    ,   mefdn::nontype<
            transfer<T*> (*)(context<T*>, Arg*)
        ,   Func
        >                       /*func*/
    ,   Arg* const              arg
    )
    {
        return mectx::save_context<T, Arg, Func>(sp, size, arg);
    }
    
    template <typename T, typename Arg, transfer<T*> (*Func)(context<T*>, Arg*)>
    static transfer<T*> swap_context(
        const context<T*>       ctx
    ,   mefdn::nontype<
            transfer<T*> (*)(context<T*>, Arg*)
        ,   Func
        >                       /*func*/
    ,   Arg* const              arg
    )
    {
        return mectx::swap_context<T, Arg, Func>(ctx, arg);
    }

    // restore_context

    template <typename T, typename Arg, transfer<T*> (*Func)(Arg*)>
    MEFDN_NORETURN
    static void restore_context(
        const context<T*>        ctx
    ,   mefdn::nontype<
            transfer<T*> (*)(Arg*)
        ,   Func
        >                       /*func*/
    ,   Arg* const              arg
    )
    {
        mectx::restore_context<T, Arg, Func>(ctx, arg);
    }
};

} // namespace mectx
} // namespace menps

