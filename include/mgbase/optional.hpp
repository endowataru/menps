
#pragma once

#include <mgbase/type_traits.hpp>
#include <mgbase/assert.hpp>

namespace mgbase {

struct nullopt_t {
    // See also: http://stackoverflow.com/questions/28332078/experimentaloptional-nullopt-t-constructor
    MGBASE_CONSTEXPR_FUNCTION nullopt_t(int) { }
};

extern nullopt_t nullopt; // never defined

// FIXME: This class works correctly only with POD types...

template <typename T>
class optional
{
public:
    MGBASE_CONSTEXPR_CPP14 optional() MGBASE_NOEXCEPT {
        create_empty();
    }
    MGBASE_CONSTEXPR_CPP14 /*implicit*/ optional(nullopt_t) MGBASE_NOEXCEPT {
        create_empty();
    }
    
    optional(const optional& other) {
        if (other)
            create(*other);
        else
            create_empty();
    }
    
    /*implicit*/ optional(const T& value) { create(value); }
    
    // TODO: replace with safe-bool idioms
    MGBASE_CONSTEXPR_FUNCTION MGBASE_EXPLICIT_OPERATOR bool () const MGBASE_NOEXCEPT {
        return engaged_;
    }
    
    MGBASE_CONSTEXPR_CPP14 T* operator -> () MGBASE_NOEXCEPT {
        MGBASE_ASSERT(engaged_);
        return &value_;
    }
    MGBASE_CONSTEXPR_CPP14 const T* operator -> () const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(engaged_);
        return &value_;
    }
    
    MGBASE_CONSTEXPR_CPP14 T& operator * () MGBASE_NOEXCEPT {
        MGBASE_ASSERT(engaged_);
        return value_;
    }
    MGBASE_CONSTEXPR_CPP14 const T& operator * () const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(engaged_);
        return value_;
    }

private:
    template <typename Arg>
    void create(Arg arg) {
        engaged_ = true;
        value_ = arg;
    }
    
    void create_empty() MGBASE_NOEXCEPT {
        engaged_ = false;
    }
    
    bool engaged_;
    T value_;
};

template <typename T>
MGBASE_CONSTEXPR_CPP14 optional<T> make_optional(T& value) {
    return optional<T>(value);
}
template <typename T>
MGBASE_CONSTEXPR_CPP14 optional<T> make_optional(const T& value) {
    return optional<T>(value);
}

} // namespace mgbase

