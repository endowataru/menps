
#pragma once

#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meult {

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

fcontext_t meult_make_fcontext(
    void*                   sp
,   mefdn::size_t          size
,   make_fcontext_func_t    func
)
noexcept;

fcontext_transfer_t meult_jump_fcontext(
    fcontext_t  fctx
,   void*       data
);

fcontext_transfer_t meult_ontop_fcontext(
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
,   const mefdn::size_t        size
,   void (* const func)(fcontext_argument<B, A>)
)
noexcept
{
    MEFDN_LOG_INFO(
        "msg:Make new context.\t"
        "sp:{:x}\t"
        "size:{}\t"
        "func:{:x}"
    ,   reinterpret_cast<mefdn::uintptr_t>(sp)
    ,   size
    ,   reinterpret_cast<mefdn::uintptr_t>(func)
    );
    
    const auto f = reinterpret_cast<make_fcontext_func_t>(func);
    
    return { detail::meult_make_fcontext(sp, size, f) };
}

template <typename C, typename B, typename A>
inline fcontext_result<C, B> jump_fcontext(
    const fcontext<B, A>    fctx
,   A* const                data
) {
    MEFDN_LOG_INFO(
        "msg:Jump to another context.\t"
        "fctx:{:x}\t"
        "data:{:x}"
    ,   reinterpret_cast<mefdn::uintptr_t>(fctx.fctx)
    ,   reinterpret_cast<mefdn::uintptr_t>(data)
    );
    
    const auto r = detail::meult_jump_fcontext(fctx.fctx, data);
    
    return { { r.fctx }, static_cast<B*>(r.data) };
}

template <typename C, typename B, typename A, typename T>
inline fcontext_result<C, A> ontop_fcontext(
    const fcontext<B, A>    fctx
,   T* const                data
,   fcontext_result<C, A>   (* const func)(fcontext_argument<B, T>)
) {
    MEFDN_LOG_INFO(
        "msg:Jump to another context and call function.\t"
        "fctx:{:x}\t"
        "data:{:x}\t"
        "func:{:x}"
    ,   reinterpret_cast<mefdn::uintptr_t>(fctx.fctx)
    ,   reinterpret_cast<mefdn::uintptr_t>(data)
    ,   reinterpret_cast<mefdn::uintptr_t>(func)
    );
    
    const auto f = reinterpret_cast<ontop_fcontext_func_t>(func);
    
    const auto r = detail::meult_ontop_fcontext(fctx.fctx, data, f);
    
    return { { r.fctx }, static_cast<B*>(r.data) };
}

} // namespace meult
} // namespace menps

