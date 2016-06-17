
#pragma once

#include <mgbase/type_traits.hpp>

#if !defined(MGBASE_ENABLE_EMULATE_MOVE) && !defined(MGBASE_CXX11_SUPPORTED)
    #define MGBASE_ENABLE_EMULATE_MOVE
#endif

#ifdef MGBASE_ENABLE_EMULATE_MOVE

namespace mgbase {

namespace detail {

struct move_nat { };

template <typename T, typename U>
MGBASE_ALWAYS_INLINE T move_to_rv_cast(const U arg) MGBASE_NOEXCEPT {
    return static_cast<T>(arg);
}

} // namespace detail


// rv

#ifdef MGBASE_COMPILER_GCC_SUPPORTS_PRAGMA_DIAGNOSTIC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
        // warning: ‘class mgbase::rv<T>’ only defines a private destructor and has no friends [-Wctor-dtor-privacy]
#endif

template <typename T>
class rv
    : public mgbase::conditional<
        mgbase::is_class<T>::value
    ,   T
    ,   detail::move_nat
    >::type
{
private:
    // never defined
    rv();
    ~rv();
    
}
MGBASE_MAY_ALIAS;

#ifdef MGBASE_COMPILER_GCC_SUPPORTS_PRAGMA_DIAGNOSTIC
    #pragma GCC diagnostic pop
#endif

// is_rv

namespace detail {

template <typename T>
struct is_rv : false_type { };

template <typename T>
struct is_rv< rv<T> > : true_type { };

template <typename T>
struct is_rv< const rv<T> > : true_type { };

} // namespace detail


// MGBASE_MOVABLE_BUT_NOT_COPYABLE

#define MGBASE_MOVABLE_BUT_NOT_COPYABLE(TYPE)   \
    private: \
        /* Because rv<T> must be prioritized, this constructor takes TYPE& */ \
        TYPE(TYPE&); \
        \
        TYPE& operator = (TYPE&); \
    \
    public: \
        MGBASE_ALWAYS_INLINE operator ::mgbase::rv<TYPE>&() MGBASE_NOEXCEPT { \
            return * ::mgbase::detail::move_to_rv_cast< ::mgbase::rv< TYPE >*>(this); \
        } \
        MGBASE_ALWAYS_INLINE operator const ::mgbase::rv<TYPE>&() const MGBASE_NOEXCEPT { \
            return * ::mgbase::detail::move_to_rv_cast<const ::mgbase::rv< TYPE >*>(this); \
        }

// MGBASE_RV_REF

#define MGBASE_RV_REF(...) ::mgbase::rv< __VA_ARGS__ >&

} // namespace mgbase

// MGBASE_MOVE_RET

#define MGBASE_MOVE_RET(type, val)  (::mgbase::move(val))

#else

#include <utility>

#define MGBASE_MOVABLE_BUT_NOT_COPYABLE(TYPE)   \
    public: \
        TYPE(const TYPE&) = delete; \
        TYPE& operator = (TYPE&) = delete;

#define MGBASE_RV_REF(...) __VA_ARGS__ &&

namespace mgbase {

using std::move;
using std::forward;

} // namespace mgbase

#define MGBASE_MOVE_RET(type, val)  (val)

#endif

