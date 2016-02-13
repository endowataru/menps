
#pragma once

#include "deferred.hpp"

namespace mgbase {

namespace detail {

template <typename T>
struct set_terminal_handler
{
    static MGBASE_ALWAYS_INLINE resumable assign(T& dest, const value_wrapper<T>& df) MGBASE_NOEXCEPT {
        dest = df.get();
        return make_empty_resumable();
    }
};

template <>
struct set_terminal_handler<void>
{
    static MGBASE_ALWAYS_INLINE resumable assign(const value_wrapper<void>& /*df*/) MGBASE_NOEXCEPT {
        return make_empty_resumable();
    }
};

template <typename Derived, typename T>
resumable deferred_base<Derived, T>::set_terminal(T* dest)
{
    continuation<T>* const current_cont = derived().get_continuation();

    if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
    {
        *dest = derived().to_ready().get();
        return make_empty_resumable();
    }
    else
    {
        derived().set_continuation(
            mgbase::make_callback_function(
                mgbase::bind1st_of_2(
                    MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(&set_terminal_handler<T>::assign)
                ,   mgbase::wrap_reference(*dest)
                )
            )
        );
        
        MGBASE_ASSERT(!derived().get_resumable().empty());
        
        return derived().get_resumable();
    }
}

template <typename Derived>
resumable deferred_base<Derived, void>::set_terminal()
{
    continuation<void>* const current_cont = derived().get_continuation();

    if (MGBASE_LIKELY(current_cont == MGBASE_NULLPTR))
    {
        return make_empty_resumable();
    }
    else
    {
        derived().set_continuation(
            mgbase::make_callback_function(
                MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(&set_terminal_handler<void>::assign)
            )
        );
        
        MGBASE_ASSERT(!derived().get_resumable().empty());
        
        return derived().get_resumable();
    }
}

} // namespace detail

} // namespace mgbase

