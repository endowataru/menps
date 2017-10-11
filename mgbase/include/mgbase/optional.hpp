
#pragma once

#include <mgbase/type_traits.hpp>
#include <mgbase/assert.hpp>

namespace mgbase {

struct nullopt_t {
    // See also: http://stackoverflow.com/questions/28332078/experimentaloptional-nullopt-t-constructor
    MGBASE_CONSTEXPR nullopt_t(int) { }
};

//extern nullopt_t nullopt; // never defined

namespace /*unnamed*/ {

    nullopt_t nullopt(0);

} // unnamed namespace

template <typename T>
class optional
{
public:
    MGBASE_CONSTEXPR_CXX14 optional() MGBASE_NOEXCEPT {
        create_empty();
    }
    MGBASE_CONSTEXPR_CXX14 /*implicit*/ optional(nullopt_t) MGBASE_NOEXCEPT {
        create_empty();
    }
    
    optional(const optional& other) {
        if (other)
            create(*other);
        else
            create_empty();
    }
    
    ~optional() { destroy(); }
    
    /*implicit*/ optional(const T& val) { create(val); }
    
    optional& operator = (const optional& other) {
        if (other)
            assign(*other);
        else
            destroy();
        
        return *this;
    }
    optional& operator = (nullopt_t) {
        destroy();
        return *this;
    }
    optional& operator = (const T& val) {
        assign(val);
        return *this;
    }
    
    // TODO: replace with safe-bool idioms
    MGBASE_CONSTEXPR MGBASE_EXPLICIT_OPERATOR bool () const MGBASE_NOEXCEPT {
        return engaged_;
    }
    
    MGBASE_CONSTEXPR_CXX14 T* operator -> () MGBASE_NOEXCEPT {
        MGBASE_ASSERT(engaged_);
        return &value();
    }
    MGBASE_CONSTEXPR_CXX14 const T* operator -> () const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(engaged_);
        return &value();
    }
    
    MGBASE_CONSTEXPR_CXX14 T& operator * () MGBASE_NOEXCEPT {
        MGBASE_ASSERT(engaged_);
        return value();
    }
    MGBASE_CONSTEXPR_CXX14 const T& operator * () const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(engaged_);
        return value();
    }

private:
    T& value() MGBASE_NOEXCEPT {
        return *reinterpret_cast<T*>(&storage_);
    }
    const T& value() const MGBASE_NOEXCEPT {
        return *reinterpret_cast<const T*>(&storage_);
    }
    
    template <typename Arg>
    void create(Arg& arg) {
        engaged_ = true;
        
        // Call the constructor.
        new (&storage_) T(arg);
    }
    template <typename Arg>
    void create(const Arg& arg) {
        engaged_ = true;
        
        // Call the constructor.
        new (&storage_) T(arg);
    }
    
    void create_empty() MGBASE_NOEXCEPT {
        engaged_ = false;
    }
    
    void destroy()
    {
        if (engaged_) {
            // Call the destructor.
            (*this)->~T();
            
            engaged_ = false;
        }
    }
    
    template <typename Arg>
    void assign(Arg& arg)
    {
        if (engaged_)
            value() = arg;
        else
            create(arg);
    }
    
    typedef typename mgbase::aligned_storage<
        sizeof(T)
    ,   mgbase::alignment_of<T>::value
    >::type
    storage_type;
    
    MGBASE_STATIC_ASSERT(sizeof(storage_type) >= sizeof(T));
    MGBASE_STATIC_ASSERT(mgbase::alignment_of<storage_type>::value >= mgbase::alignment_of<T>::value);
    
    bool engaged_;
    storage_type storage_;
};

template <typename T>
MGBASE_ALWAYS_INLINE MGBASE_CONSTEXPR_CXX14
optional<T> make_optional(T& value) {
    return optional<T>(value);
}
template <typename T>
MGBASE_ALWAYS_INLINE MGBASE_CONSTEXPR_CXX14
optional<T> make_optional(const T& value) {
    return optional<T>(value);
}

} // namespace mgbase

