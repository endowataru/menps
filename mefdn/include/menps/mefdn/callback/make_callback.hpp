
#pragma once

#include "callback.hpp"
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/nontype.hpp>

namespace menps {
namespace mefdn {

// Originally in <mgbase/atomic/atomic_base.hpp>
#define MEFDN_FETCH_OP_LIST(x) \
    x(add, ADD, +) \
    x(sub, SUB, -) \
    x(and, AND, &) \
    x(xor, XOR, ^) \
    x(or , OR , |)

namespace detail {

template <typename T, typename Value, typename MemoryOrder>
struct store_callback
{
    volatile mefdn::atomic<T>* obj;
    
    void operator () () const {
        obj->store(Value{}(), MemoryOrder{}());
    }
};

#define DEFINE_EXECUTE_FETCH_OP(name, NAME, c_op) \
template <typename T, typename Value, typename MemoryOrder> \
struct fetch_##name##_callback \
{ \
    volatile mefdn::atomic<T>* obj; \
    \
    void operator () () const { \
        obj->fetch_##name(Value{}(), MemoryOrder{}()); \
    } \
};

MEFDN_FETCH_OP_LIST(DEFINE_EXECUTE_FETCH_OP)

#undef DEFINE_EXECUTE_FETCH_OP

} // namespace detail

template <typename T, typename Value, typename MemoryOrder>
inline callback<void ()> make_callback_store(
    volatile mefdn::atomic<T>* const obj
,   Value /*ignored*/
,   MemoryOrder /*ignored*/
) noexcept
{
    return callback<void ()>( detail::store_callback<T, Value, MemoryOrder>{ obj } );
}

template <typename T, typename Value>
inline callback<void ()> make_callback_store_release(
    volatile mefdn::atomic<T>* const obj
,   Value /*ignored*/
) noexcept
{
    return make_callback_store(obj, Value{}, MEFDN_NONTYPE(memory_order_release));
}

#define DEFINE_MAKE_CALLBACK_FETCH_OP(name, NAME, c_op) \
template <typename T, typename Value, typename MemoryOrder> \
inline callback<void ()> make_callback_fetch_##name( \
    volatile mefdn::atomic<T>* const obj \
,   Value /*ignored*/ \
,   MemoryOrder /*ignored*/ \
) noexcept \
{ \
    return callback<void ()>( detail::fetch_##name##_callback<T, Value, MemoryOrder>{ obj } ); \
} \
\
template <typename T, typename Value> \
inline callback<void ()> make_callback_fetch_##name##_release( \
    volatile mefdn::atomic<T>* const obj \
,   Value /*ignored*/ \
) noexcept \
{ \
    return make_callback_fetch_##name(obj, Value{}, MEFDN_NONTYPE(memory_order_release)); \
}

MEFDN_FETCH_OP_LIST(DEFINE_MAKE_CALLBACK_FETCH_OP)

#undef DEFINE_MAKE_CALLBACK_FETCH_OP


namespace detail {

template <typename SyncFlag>
struct notify_callback
{
    SyncFlag* flag;
    
    void operator () () const {
        flag->notify();
    }
};

} // namespace detail

template <typename SyncFlag>
inline callback<void ()> make_callback_notify(
    SyncFlag* flag
) noexcept
{
    return callback<void ()>( detail::notify_callback<SyncFlag>{ flag } );
}

namespace detail {

struct empty_callback
{
    void operator() () const { }
};

} // namespace detail

inline callback<void ()> make_callback_empty()
{
    return callback<void ()>(detail::empty_callback{});
}

} // namespace mefdn
} // namespace menps

