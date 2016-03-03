
#pragma once

#include <mgbase/lang.hpp>

// Reference: Boost.Core

namespace mgbase {

#ifdef MGBASE_CPP11_SUPPORTED

template <typename EnumType>
struct native_type {
    typedef EnumType    EnumType;
};

template <typename EnumType>
MGBASE_ALWAYS_INLINE constexpr
EnumType native_value(const EnumType e) MGBASE_NOEXCEPT {
    return e;
}

#define MGBASE_SCOPED_ENUM_UT_DECLARE_BEGIN(EnumType, UnderlyingType) \
    enum class EnumType : UnderlyingType

#define MGBASE_SCOPED_ENUM_DECLARE_BEGIN(EnumType) \
    enum class EnumType

#define MGBASE_SCOPED_ENUM_DECLARE_END(EnumType) ;

#else

/// Meta-function to get the native enum type.
template <typename EnumType>
struct native_type {
    typedef typename EnumType::enum_type    type;
};

/// Cast a scoped enum to its native enum type.
template <typename EnumType>
MGBASE_ALWAYS_INLINE MGBASE_CONSTEXPR
typename EnumType::enum_type native_value(const EnumType e) MGBASE_NOEXCEPT {
    return e.get_native_value_();
}

#define MGBASE_SCOPED_ENUM_UT_DECLARE_BEGIN(EnumType, UnderlyingType)       \
    struct EnumType                                                         \
    {                                                                       \
        typedef UnderlyingType  underlying_type;                            \
                                                                            \
        EnumType() MGBASE_NOEXCEPT {}                                       \
                                                                            \
        explicit MGBASE_CONSTEXPR                                           \
        EnumType(const underlying_type val) MGBASE_NOEXCEPT : val_(val) {}  \
                                                                            \
        MGBASE_CONSTEXPR underlying_type                                    \
        get_underlying_value_() const MGBASE_NOEXCEPT { return val_; }      \
                                                                            \
    private:                                                                \
        underlying_type val_;                                               \
        typedef EnumType self_type;                                         \
                                                                            \
    public:                                                                 \
        enum enum_type

#define MGBASE_SCOPED_ENUM_DECLARE_BEGIN(EnumType)  \
    MGBASE_SCOPED_ENUM_UT_DECLARE_BEGIN(EnumType, int)

#define MGBASE_SCOPED_ENUM_DECLARE_END(EnumType)                                                                                                        \
        ;                                                                                                                                               \
                                                                                                                                                        \
        MGBASE_CONSTEXPR EnumType(const enum_type val) MGBASE_NOEXCEPT : val_(val) { }                                                                  \
                                                                                                                                                        \
        MGBASE_CONSTEXPR enum_type get_native_value_() const MGBASE_NOEXCEPT { return enum_type(val_); }                                                \
                                                                                                                                                        \
        friend MGBASE_CONSTEXPR bool operator == (self_type lhs, self_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)==enum_type(rhs.val_); }    \
        friend MGBASE_CONSTEXPR bool operator == (self_type lhs, enum_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)==rhs; }                    \
        friend MGBASE_CONSTEXPR bool operator == (enum_type lhs, self_type rhs) MGBASE_NOEXCEPT { return lhs==enum_type(rhs.val_); }                    \
        friend MGBASE_CONSTEXPR bool operator != (self_type lhs, self_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)!=enum_type(rhs.val_); }    \
        friend MGBASE_CONSTEXPR bool operator != (self_type lhs, enum_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)!=rhs; }                    \
        friend MGBASE_CONSTEXPR bool operator != (enum_type lhs, self_type rhs) MGBASE_NOEXCEPT { return lhs!=enum_type(rhs.val_); }                    \
        friend MGBASE_CONSTEXPR bool operator <  (self_type lhs, self_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)<enum_type(rhs.val_); }     \
        friend MGBASE_CONSTEXPR bool operator <  (self_type lhs, enum_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)<rhs; }                     \
        friend MGBASE_CONSTEXPR bool operator <  (enum_type lhs, self_type rhs) MGBASE_NOEXCEPT { return lhs<enum_type(rhs.val_); }                     \
        friend MGBASE_CONSTEXPR bool operator <= (self_type lhs, self_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)<=enum_type(rhs.val_); }    \
        friend MGBASE_CONSTEXPR bool operator <= (self_type lhs, enum_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)<=rhs; }                    \
        friend MGBASE_CONSTEXPR bool operator <= (enum_type lhs, self_type rhs) MGBASE_NOEXCEPT { return lhs<=enum_type(rhs.val_); }                    \
        friend MGBASE_CONSTEXPR bool operator >  (self_type lhs, self_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)>enum_type(rhs.val_); }     \
        friend MGBASE_CONSTEXPR bool operator >  (self_type lhs, enum_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)>rhs; }                     \
        friend MGBASE_CONSTEXPR bool operator >  (enum_type lhs, self_type rhs) MGBASE_NOEXCEPT { return lhs>enum_type(rhs.val_); }                     \
        friend MGBASE_CONSTEXPR bool operator >= (self_type lhs, self_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)>=enum_type(rhs.val_); }    \
        friend MGBASE_CONSTEXPR bool operator >= (self_type lhs, enum_type rhs) MGBASE_NOEXCEPT { return enum_type(lhs.val_)>=rhs; }                    \
        friend MGBASE_CONSTEXPR bool operator >= (enum_type lhs, self_type rhs) MGBASE_NOEXCEPT { return lhs>=enum_type(rhs.val_); }                    \
    };

#endif

} // namespace mgbase

