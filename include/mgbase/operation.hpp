
#pragma once

#include <mgbase/operation.h>
#include <mgbase/callback_function.hpp>
#include <mgbase/atomic.hpp>
#include <mgbase/logger.hpp>

namespace mgbase {

typedef mgbase_operation_code     operation_code;
typedef mgbase_operation          operation;
typedef mgbase_operation_operands operation_operands;
typedef mgbase_operation_argument operation_argument;

namespace detail {

namespace /*unnamed*/ {

// execute

// TODO: Strictly speaking, each atomic type is different
// e.g. atomic<short> vs. atomic<uint16_t>

template <typename T>
MGBASE_ALWAYS_INLINE void execute_store_release(const operation& opr) MGBASE_NOEXCEPT
{
    const operation_operands& operands = opr.arg.operands;
    
    volatile mgbase::atomic<T>* const target
        = static_cast<volatile mgbase::atomic<T>*>(operands.pointer);
    
    const T value = static_cast<T>(operands.value);
    
    target->store(value, mgbase::memory_order_release);
}

#define DEFINE_EXECUTE_FETCH_OP(op, OP) \
template <typename T> \
MGBASE_ALWAYS_INLINE void execute_fetch_##op(const operation& opr) MGBASE_NOEXCEPT \
{ \
    const operation_operands& operands = opr.arg.operands; \
    \
    volatile mgbase::atomic<T>* const target \
        = static_cast<volatile mgbase::atomic<T>*>(operands.pointer); \
    \
    const T value = static_cast<T>(operands.value); \
    \
    target->fetch_##op(value, mgbase::memory_order_release); \
}

MGBASE_FETCH_OP_LIST(DEFINE_EXECUTE_FETCH_OP)

#undef DEFINE_EXECUTE_FETCH_OP


// make_operation

MGBASE_ALWAYS_INLINE operation_argument make_operands(volatile void* ptr, const mgbase::uint64_t val) {
    const operation_operands opr = { ptr, val };
    operation_argument result;
    result.operands = opr;
    return result;
}

template <typename T>
MGBASE_ALWAYS_INLINE operation_code get_store_code(volatile T* const /*obj*/) {
    switch (sizeof(T)) {
        case 1:     return MGBASE_OPERATION_STORE_RELEASE_INT8;
        case 2:     return MGBASE_OPERATION_STORE_RELEASE_INT16;
        case 4:     return MGBASE_OPERATION_STORE_RELEASE_INT32;
        case 8:     return MGBASE_OPERATION_STORE_RELEASE_INT64;
        default:    MGBASE_UNREACHABLE();
    }
}

#define DEFINE_FETCH_OP(op, OP) \
template <typename T> \
MGBASE_ALWAYS_INLINE operation_code get_fetch_##op##_code(volatile T* const /*obj*/) { \
    switch (sizeof(T)) { \
        case 1:     return MGBASE_OPERATION_FETCH_##OP##_INT8 ; \
        case 2:     return MGBASE_OPERATION_FETCH_##OP##_INT16; \
        case 4:     return MGBASE_OPERATION_FETCH_##OP##_INT32; \
        case 8:     return MGBASE_OPERATION_FETCH_##OP##_INT64; \
        default:    MGBASE_UNREACHABLE(); \
    } \
}

MGBASE_FETCH_OP_LIST(DEFINE_FETCH_OP)

#undef DEFINE_FETCH_OP
} // unnamed namespace

} // namespace detail

namespace /*unnamed*/ {

MGBASE_ALWAYS_INLINE void execute(const operation& opr) MGBASE_NOEXCEPT
{
    MGBASE_ASSERT(0 <= opr.code && opr.code < MGBASE_OPERATION_END);
    
    MGBASE_LOG_DEBUG(
        "msg:Executing operation.\t"
        "code:{}\tpointer:{:x}\tvalue:{}"
    ,   opr.code
    ,   reinterpret_cast<mgbase::intptr_t>(opr.arg.operands.pointer)
    ,   opr.arg.operands.value
    );
    
    switch (opr.code)
    {
        case MGBASE_OPERATION_NO_OPERATION:
            // Do nothing.
            break;
        
        case MGBASE_OPERATION_CALL_CALLBACK_FUNCTION:
            opr.arg.func();
            break;
        
        case MGBASE_OPERATION_STORE_RELEASE_INT8:   detail::execute_store_release<mgbase::int8_t> (opr);    break;
        case MGBASE_OPERATION_STORE_RELEASE_INT16:  detail::execute_store_release<mgbase::int16_t>(opr);    break;
        case MGBASE_OPERATION_STORE_RELEASE_INT32:  detail::execute_store_release<mgbase::int32_t>(opr);    break;
        case MGBASE_OPERATION_STORE_RELEASE_INT64:  detail::execute_store_release<mgbase::int64_t>(opr);    break;
        
        #define DEFINE_FETCH_OP_CASE(op, OP)                                                                       \
        case MGBASE_OPERATION_FETCH_##OP##_INT8:    detail::execute_fetch_##op<mgbase::int8_t >(opr);       break; \
        case MGBASE_OPERATION_FETCH_##OP##_INT16:   detail::execute_fetch_##op<mgbase::int16_t>(opr);       break; \
        case MGBASE_OPERATION_FETCH_##OP##_INT32:   detail::execute_fetch_##op<mgbase::int32_t>(opr);       break; \
        case MGBASE_OPERATION_FETCH_##OP##_INT64:   detail::execute_fetch_##op<mgbase::int64_t>(opr);       break;
        
        MGBASE_FETCH_OP_LIST(DEFINE_FETCH_OP_CASE)
        
        #undef DEFINE_FETCH_OP_CASE
        
        case MGBASE_OPERATION_END:
            MGBASE_UNREACHABLE();
            break;
    }
}


MGBASE_ALWAYS_INLINE operation make_no_operation() MGBASE_NOEXCEPT {
    const operation result = { MGBASE_OPERATION_NO_OPERATION, detail::make_operands(MGBASE_NULLPTR, 0) };
    return result;
}

template <typename A, typename C>
MGBASE_ALWAYS_INLINE operation make_operation_store_release(volatile A* const obj, const C val) {
    const operation result = { detail::get_store_code(obj), detail::make_operands(obj, val) };
    return result;
}
template <typename T>
MGBASE_ALWAYS_INLINE operation make_operation_store_release(volatile mgbase::atomic<T>* const obj, const T val) {
    const operation result = { detail::get_store_code(obj), detail::make_operands(obj, val) };
    return result;
}

#define DEFINE_MAKE_OPERATION(op, OP)   \
template <typename A, typename C> \
MGBASE_ALWAYS_INLINE operation make_operation_fetch_##op##_release(volatile A* const obj, const C val) MGBASE_NOEXCEPT { \
    operation result = { \
        detail::get_fetch_##op##_code(obj) \
    ,   detail::make_operands(obj, static_cast<mgbase::uint64_t>(val)) \
    }; \
    return result; \
} \
template <typename T> \
MGBASE_ALWAYS_INLINE operation make_operation_fetch_##op##_release(volatile mgbase::atomic<T>* const obj, const T val) MGBASE_NOEXCEPT { \
    operation result = { \
        detail::get_fetch_##op##_code(obj) \
    ,   detail::make_operands(obj, static_cast<mgbase::uint64_t>(val)) \
    }; \
    return result; \
}

MGBASE_FETCH_OP_LIST(DEFINE_MAKE_OPERATION)

#undef DEFINE_MAKE_OPERATION

template <typename Signature>
MGBASE_ALWAYS_INLINE operation make_operation_call(const callback_function<Signature>& func) MGBASE_NOEXCEPT {
    operation_argument arg;
    arg.func = func;
    const operation result = { MGBASE_OPERATION_CALL_CALLBACK_FUNCTION, arg };
    return result;
}

} // unnamed namespace

} // namespace mgbase

