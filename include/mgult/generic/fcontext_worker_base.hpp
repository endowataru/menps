
#pragma once

#include <mgult/fcontext.hpp>

namespace mgult {

struct fcontext_worker_traits_base
{
    template <typename B, typename A>
    struct context {
        typedef fcontext<B, A>  type;
    };
    template <typename B, typename A>
    struct context_result {
        typedef fcontext_result<B, A>   type;
    };
    template <typename B, typename A>
    struct context_argument {
        typedef fcontext_argument<B, A> type;
    };
};

class fcontext_worker_base
{
public:
    template <typename B2, typename A2, typename B1, typename A1>
    static fcontext<B2, A2> cast_context(
        const fcontext<B1, A1>  fctx
    ) {
        // Ignore types.
        return { fctx.fctx };
    }
    
    template <typename B, typename A>
    static fcontext<B, A> make_context(
        void* const                 sp
    ,   const mgbase::size_t        size
    ,   void (* const func)(fcontext_argument<B, A>)
    ) {
        return make_fcontext(sp, size, func);
    }
    
    template <typename C, typename B, typename A>
    static fcontext_result<C, B> jump_context(
        const fcontext<B, A>    fctx
    ,   A* const                data
    ) {
        return jump_fcontext<C>(fctx, data);
    }
    
    template <typename C, typename B, typename A, typename T>
    static fcontext_result<C, A> ontop_context(
        const fcontext<B, A>    fctx
    ,   T* const                data
    ,   fcontext_result<C, A>   (* const func)(fcontext_argument<B, T>)
    ) {
        return ontop_fcontext(fctx, data, func);
    }
    
    template <typename C, typename B, typename A>
    static fcontext_result<C, B> jump_new_context(
        void* const             stack_ptr
    ,   const mgbase::size_t    stack_size
    ,   void                    (* const func)(fcontext_argument<B, A>)
    ,   A* const                data
    )
    {
        const auto ctx = make_fcontext(stack_ptr, stack_size, func);
        
        return jump_context<C>(ctx, data);
    }
};

} // namespace mgult

