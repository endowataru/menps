
#pragma once

#include <mgbase/operation.h>
#include <mgbase/atomic.hpp>
#include <mgbase/callback_function.hpp>

namespace mgbase {

typedef mgbase_operation    operation;

namespace detail {

MGBASE_ALWAYS_INLINE void execute_empty(const operation&) MGBASE_NOEXCEPT { }

template <typename T>
MGBASE_ALWAYS_INLINE void execute_store_release(
    volatile mgbase::atomic<T>& obj
,   const operation&            opr
) MGBASE_NOEXCEPT
{
    const T value = static_cast<T>(opr.value);
    
    obj.store(value, mgbase::memory_order_release);
}

#define DEFINE_EXECUTE_FETCH_OP_RELEASE(name, NAME, c_op) \
template <typename T> \
MGBASE_ALWAYS_INLINE void execute_fetch_##name##_release( \
    volatile mgbase::atomic<T>& obj \
,   const operation&            opr \
) MGBASE_NOEXCEPT \
{ \
    const T value = static_cast<T>(opr.value); \
    \
    obj.fetch_##name(value, mgbase::memory_order_release); \
}

MGBASE_FETCH_OP_LIST(DEFINE_EXECUTE_FETCH_OP_RELEASE)

#undef DEFINE_EXECUTE_FETCH_OP_RELEASE

} // namespace detail

MGBASE_ALWAYS_INLINE operation make_no_operation() MGBASE_NOEXCEPT {
    const operation result = {
        mgbase::make_callback_function(
            MGBASE_MAKE_INLINED_FUNCTION(detail::execute_empty)
        )
    ,   0
    };
    return result;
}

template <typename T>
MGBASE_ALWAYS_INLINE operation make_operation_store_release(
    volatile mgbase::atomic<T>* const   obj
,   const T                             val
) MGBASE_NOEXCEPT
{
    const callback_function<void (const operation&)> func
        = mgbase::make_callback_function(
            mgbase::bind1st_of_2(
                MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(detail::execute_store_release<T>)
            ,   mgbase::wrap_reference(*obj)
            )
        );
    
    const operation result = { func, val };
    return result;
}

#define DEFINE_MAKE_OPERATION_FETCH_OP_RELEASE(name, NAME, c_op) \
template <typename T> \
MGBASE_ALWAYS_INLINE operation make_operation_fetch_##name##_release( \
    volatile mgbase::atomic<T>* const   obj \
,   const T                             val \
) MGBASE_NOEXCEPT \
{ \
    const callback_function<void (const operation&)> func \
        = mgbase::make_callback_function( \
            mgbase::bind1st_of_2( \
                MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(detail::execute_fetch_##name##_release<T>) \
            ,   mgbase::wrap_reference(*obj) \
            ) \
        ); \
    \
    const operation result = { func, val }; \
    return result; \
}

MGBASE_FETCH_OP_LIST(DEFINE_MAKE_OPERATION_FETCH_OP_RELEASE)

#undef DEFINE_MAKE_OPERATION_FETCH_OP_RELEASE

MGBASE_ALWAYS_INLINE operation make_operation_call(const callback_function<void (const operation&)>& func) MGBASE_NOEXCEPT {
    const operation result = { func, 0 };
    return result;
}


MGBASE_ALWAYS_INLINE void execute(const operation& opr)
{
    opr.func(opr);
}

} // namespace mgbase

