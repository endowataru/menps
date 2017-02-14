
#pragma once

#include <mgcom/rma/requester.hpp>
#include <mgcom/ult.hpp>

namespace mgcom {
namespace rma {

// Variations:
// - explicit requester
// - typed
// - suspend_and_call (async or sync)

template <typename T>
struct read_params
{
    process_id_t                src_proc;
    remote_ptr<const T>         src_rptr;
    local_ptr<T>                dest_lptr;
    index_t                     num_elems;
};
template <typename T>
struct write_params
{
    process_id_t                dest_proc;
    remote_ptr<T>               dest_rptr;
    local_ptr<const T>          src_lptr;
    index_t                     num_elems;
};

template <typename T>
struct async_read_params : read_params<T>
{
    mgbase::callback<void ()>   on_complete;
};
template <typename T>
struct async_write_params : write_params<T>
{
    mgbase::callback<void ()>   on_complete;
};

namespace detail {

template <typename Remote, typename Local>
inline read_params<Local> make_read_params(
    const process_id_t          src_proc
,   const remote_ptr<Remote>&   src_rptr
,   const local_ptr<Local>&     dest_lptr
,   const index_t               num_elems
) {
    // Note: Important for type safety.
    MGBASE_STATIC_ASSERT_MSG(
        (mgbase::is_same<
            typename mgbase::remove_const<Remote>::type
        ,   Local
        >::value)
    ,   "Breaking type safety"
    );
    
    return {
        src_proc
    ,   mgbase::reinterpret_pointer_cast<Local>(src_rptr)
    ,   dest_lptr
    ,   num_elems
    };
}

template <typename Remote, typename Local>
inline write_params<Remote> make_write_params(
    const process_id_t          dest_proc
,   const remote_ptr<Remote>&   dest_rptr
,   const local_ptr<Local>&     src_lptr
,   const index_t               num_elems
) {
    // Note: Important for type safety.
    MGBASE_STATIC_ASSERT_MSG(
        (mgbase::is_same<
            Remote
        ,   typename mgbase::remove_const<Local>::type
        >::value)
    ,   "Breaking type safety"
    );
    
    return {
        dest_proc
    ,   dest_rptr
    ,   mgbase::reinterpret_pointer_cast<Remote>(src_lptr)
    ,   num_elems * mgbase::runtime_size_of<Remote>()
    };
}

} // namespace detail

namespace detail {

template <
    typename Params
,   ult::async_status<void> (requester::*Method)(const Params&)
>
struct async_closure
{
    requester*  self;
    Params*     p;
    
    template <typename Cont>
    MGBASE_WARN_UNUSED_RESULT
    ult::async_status<void> operator() (Cont& cont) const /*may throw*/
    {
        p->on_complete = cont;
        return (self->*Method)(*p);
    }
};

} // namespace detail

#define DEFINE_RW(NAME, REMOTE, LOCAL) \
    namespace detail { \
        template <typename T> \
        inline untyped_##NAME##_params to_untyped_params(const NAME##_params<T>& params) \
        { \
            return { \
                params.REMOTE##_proc \
            ,   params.REMOTE##_rptr.to_address() \
            ,   params.LOCAL##_lptr.to_address() \
            ,   params.num_elems * mgbase::runtime_size_of<T>() \
            }; \
        } \
    } /* namespace detail */ \
    \
    /* async */ \
    MGBASE_WARN_UNUSED_RESULT \
    inline ult::async_status<void> async_##NAME( \
        requester&                              rqstr \
    ,   const async_untyped_##NAME##_params&    params \
    ) { \
        return rqstr.async_##NAME(params); \
    } \
    MGBASE_WARN_UNUSED_RESULT \
    inline ult::async_status<void> async_##NAME( \
        requester&                          rqstr \
    ,   const untyped_##NAME##_params&      params \
    ,   const mgbase::callback<void ()>&    on_complete \
    ) { \
        async_untyped_##NAME##_params p; \
        \
        untyped_##NAME##_params& base = p; \
        base = params; \
        \
        p.on_complete = on_complete; \
        \
        return async_##NAME( \
            rqstr \
        ,   p \
        ); \
    } \
    template <typename T> \
    MGBASE_WARN_UNUSED_RESULT \
    inline ult::async_status<void> async_##NAME( \
        requester&                          rqstr \
    ,   const NAME##_params<T>&             params \
    ,   const mgbase::callback<void ()>&    on_complete \
    ) { \
        return async_##NAME( \
            rqstr \
        ,   detail::to_untyped_params(params) \
        ,   on_complete \
        ); \
    } \
    template <typename Remote, typename Local> \
    MGBASE_WARN_UNUSED_RESULT \
    inline ult::async_status<void> async_##NAME( \
        requester&                          rqstr \
    ,   const process_id_t                  REMOTE##_proc \
    ,   const remote_ptr<Remote>&           REMOTE##_rptr \
    ,   const local_ptr<Local>&             LOCAL##_lptr \
    ,   const index_t                       num_elems \
    ,   const mgbase::callback<void ()>&    on_complete \
    ) { \
        return async_##NAME( \
            rqstr \
        ,   detail::make_##NAME##_params( \
                REMOTE##_proc \
            ,   REMOTE##_rptr \
            ,   LOCAL##_lptr \
            ,   num_elems \
            ) \
        ,   on_complete \
        ); \
    } \
    template <typename Remote, typename Local> \
    MGBASE_WARN_UNUSED_RESULT \
    inline ult::async_status<void> async_##NAME( \
        const process_id_t                  REMOTE##_proc \
    ,   const remote_ptr<Remote>&           REMOTE##_rptr \
    ,   const local_ptr<Local>&             LOCAL##_lptr \
    ,   const index_t                       num_elems \
    ,   const mgbase::callback<void ()>&    on_complete \
    ) { \
        return async_##NAME( \
            requester::get_instance() \
        ,   REMOTE##_proc \
        ,   REMOTE##_rptr \
        ,   LOCAL##_lptr \
        ,   num_elems \
        ,   on_complete \
        ); \
    } \
    \
    /* sync */ \
    inline void NAME( \
        requester&                          rqstr \
    ,   const untyped_##NAME##_params&      params \
    ) { \
        async_untyped_##NAME##_params p; \
        \
        untyped_##NAME##_params& base = p; \
        base = params; \
        \
        ult::suspend_and_call<void>( \
            detail::async_closure<\
                async_untyped_##NAME##_params \
            ,   &requester::async_##NAME \
            > \
            { &rqstr, &p } \
        ); \
    } \
    template <typename T> \
    inline void NAME( \
        requester&                          rqstr \
    ,   const NAME##_params<T>&             params \
    ) { \
        NAME( \
            rqstr \
        ,   detail::to_untyped_params(params) \
        ); \
    } \
    template <typename Remote, typename Local> \
    inline void NAME( \
        requester&                          rqstr \
    ,   const process_id_t                  REMOTE##_proc \
    ,   const remote_ptr<Remote>&           REMOTE##_rptr \
    ,   const local_ptr<Local>&             LOCAL##_lptr \
    ,   const index_t                       num_elems \
    ) { \
        NAME( \
            rqstr \
        ,   detail::make_##NAME##_params( \
                REMOTE##_proc \
            ,   REMOTE##_rptr \
            ,   LOCAL##_lptr \
            ,   num_elems \
            ) \
        ); \
    } \
    template <typename Remote, typename Local> \
    inline void NAME( \
        const process_id_t                  REMOTE##_proc \
    ,   const remote_ptr<Remote>&           REMOTE##_rptr \
    ,   const local_ptr<Local>&             LOCAL##_lptr \
    ,   const index_t                       num_elems \
    ) { \
        NAME( \
            requester::get_instance() \
        ,   REMOTE##_proc \
        ,   REMOTE##_rptr \
        ,   LOCAL##_lptr \
        ,   num_elems \
        ); \
    }

DEFINE_RW(read, src, dest)
DEFINE_RW(write, dest, src)

#undef DEFINE_RW

#define DEFINE_ATOMIC(NAME, ARGS_DEF, ARGS) \
    /* async */ \
    template <typename T> \
    MGBASE_WARN_UNUSED_RESULT \
    inline ult::async_status<void> async_##NAME( \
        requester&                          rqstr \
    ,   const async_##NAME##_params<T>&     params \
    ) { \
        return rqstr.async_##NAME(params); \
    } \
    template <typename T> \
    MGBASE_WARN_UNUSED_RESULT \
    inline ult::async_status<void> async_##NAME( \
        requester&                          rqstr \
    ,   const NAME##_params<T>&             params \
    ,   const mgbase::callback<void ()>&    on_complete \
    ) { \
        async_##NAME##_params<T> p; \
        \
        NAME##_params<T>& base = p; \
        base = params; \
        \
        p.on_complete = on_complete; \
        \
        return async_##NAME( \
            rqstr \
        ,   p \
        ); \
    } \
    template <typename T> \
    MGBASE_WARN_UNUSED_RESULT \
    inline ult::async_status<void> async_##NAME( \
        requester&                          rqstr \
    ,   ARGS_DEF \
    ,   const mgbase::callback<void ()>&    on_complete \
    ) { \
        return async_##NAME( \
            rqstr \
        ,   NAME##_params<T>{ ARGS } \
        ,   on_complete \
        ); \
    } \
    template <typename T> \
    MGBASE_WARN_UNUSED_RESULT \
    inline ult::async_status<void> async_##NAME( \
        ARGS_DEF \
    ,   const mgbase::callback<void ()>&    on_complete \
    ) { \
        return async_##NAME( \
            requester::get_instance() \
        ,   ARGS \
        ,   on_complete \
        ); \
    } \
    \
    /* sync */ \
    template <typename T> \
    inline void NAME( \
        requester&                          rqstr \
    ,   const NAME##_params<T>&             params \
    ) { \
        async_##NAME##_params<T> p; \
        \
        NAME##_params<T>& base = p; \
        base = params; \
        \
        ult::suspend_and_call<void>( \
            detail::async_closure<\
                async_##NAME##_params<T> \
            ,   &requester::async_##NAME \
            > \
            { &rqstr, &p } \
        ); \
    } \
    template <typename T> \
    inline void NAME( \
        requester&                          rqstr \
    ,   ARGS_DEF \
    ) { \
        NAME( \
            rqstr \
        ,   NAME##_params<T>{ ARGS } \
        ); \
    } \
    template <typename T> \
    inline void NAME( \
        ARGS_DEF \
    ) { \
        NAME( \
            requester::get_instance() \
        ,   ARGS \
        ); \
    } \


#define COMMA ,

DEFINE_ATOMIC(atomic_read,
    const process_id_t          src_proc COMMA
    const remote_ptr<const T>&  src_rptr COMMA
    T* const                    dest_ptr
,   src_proc COMMA
    src_rptr COMMA
    dest_ptr
)
DEFINE_ATOMIC(atomic_write,
    const process_id_t          dest_proc COMMA
    const remote_ptr<T>&        dest_rptr COMMA
    const T                     value
,   dest_proc COMMA
    dest_rptr COMMA
    value
)

DEFINE_ATOMIC(compare_and_swap,
    const process_id_t      target_proc COMMA
    const remote_ptr<T>&    target_rptr COMMA
    const T                 expected_val COMMA
    const T                 desired_val COMMA
    T* const                result_ptr
,   target_proc COMMA
    target_rptr COMMA
    expected_val COMMA
    desired_val COMMA
    result_ptr
)
DEFINE_ATOMIC(fetch_and_add,
    const process_id_t      target_proc COMMA
    const remote_ptr<T>&    target_rptr COMMA
    const T                 value COMMA
    T* const                result_ptr
,   target_proc COMMA
    target_rptr COMMA
    value COMMA
    result_ptr
)

#undef DEFINE_ATOMIC
#undef COMMA

} // namespace rma
} // namespace mgcom

