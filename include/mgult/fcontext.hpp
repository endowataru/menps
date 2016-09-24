
#pragma once

#include <mgbase/lang.hpp>

namespace mgult {

struct fcontext_;

typedef fcontext_*   fcontext_t;

struct fcontext_transfer_t {
    fcontext_t  fctx;
    void*       data;
};

typedef void (*make_fcontext_func_t)(fcontext_transfer_t);

typedef fcontext_transfer_t (*ontop_fcontext_func_t)(fcontext_transfer_t);

namespace detail {

extern "C" {

fcontext_t mgult_make_fcontext(
    void*                   sp
,   mgbase::size_t          size
,   make_fcontext_func_t    func
)
MGBASE_NOEXCEPT;

fcontext_transfer_t mgult_jump_fcontext(
    fcontext_t  fctx
,   void*       data
);

fcontext_transfer_t mgult_ontop_fcontext(
    fcontext_t              fctx
,   void*                   data
,   ontop_fcontext_func_t   func
);

} // extern "C"

} // namespace detail


template <typename B, typename A>
struct fcontext {
    fcontext_t  fctx;
};

template <typename B, typename A>
struct fcontext_argument {
    fcontext<void, B>   fctx;
    A*                  data;
};

template <typename B, typename A>
struct fcontext_result {
    fcontext<void, B>   fctx;
    A*                  data;
};

template <typename B, typename A>
inline fcontext<B, A> make_fcontext(
    void* const                 sp
,   const mgbase::size_t        size
,   void (* const func)(fcontext_argument<B, A>)
)
MGBASE_NOEXCEPT
{
    const auto f = reinterpret_cast<make_fcontext_func_t>(func);
    
    return { detail::mgult_make_fcontext(sp, size, f) };
}

template <typename C, typename B, typename A>
inline fcontext_result<C, B> jump_fcontext(
    const fcontext<B, A>    fctx
,   A* const                data
) {
    const auto r = detail::mgult_jump_fcontext(fctx.fctx, data);
    
    return { { r.fctx }, static_cast<B*>(r.data) };
}

template <typename C, typename B, typename A, typename T>
inline fcontext_result<C, A> ontop_fcontext(
    const fcontext<B, A>    fctx
,   T* const                data
,   fcontext_result<C, A>   (* const func)(fcontext_argument<B, T>)
) {
    const auto f = reinterpret_cast<ontop_fcontext_func_t>(func);
    
    const auto r = detail::mgult_ontop_fcontext(fctx.fctx, data, f);
    
    return { { r.fctx }, static_cast<B*>(r.data) };
}

} // namespace mgult

