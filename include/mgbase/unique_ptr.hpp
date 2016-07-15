
#pragma once

#include <mgbase/lang.hpp>

#if defined(MGBASE_CXX11_UNIQUE_PTR_SUPPORTED) && !defined(MGBASE_DISABLE_STANDARD_UNIQUE_PTR)

#include <memory>

namespace mgbase {

using std::default_delete;
using std::unique_ptr;

} // namespace mgbase

#else

// Reference: <tuple> in libstdc++

#include <mgbase/type_traits.hpp>
#include <tuple>

namespace mgbase {

template <typename T>
struct default_delete {
    default_delete() MGBASE_NOEXCEPT = default;
    
    template <typename U>
    default_delete(const default_delete<U>&) { }
    
    void operator() (T* const ptr) const {
        MGBASE_STATIC_ASSERT(sizeof(T) > 0); // can't delete pointer to incomplete type
        delete ptr;
    }
};

template <typename T>
struct default_delete<T []> {
    void operator() (T* const ptr) const {
        MGBASE_STATIC_ASSERT(sizeof(T) > 0); // can't delete pointer to incomplete type
        delete[] ptr;
    }
};


template <typename T, typename TD = default_delete<T>>
class unique_ptr
{
public:
    typedef T*  pointer;
    typedef T   element_type;
    typedef TD  deleter_type;
    
private:
    typedef std::tuple<pointer, deleter_type>   tuple_type;
    typedef tuple_type unique_ptr::*            safe_bool_type;
    
public: 
    unique_ptr() MGBASE_NOEXCEPT
        : t_{MGBASE_NULLPTR, deleter_type{}}
    {
        MGBASE_STATIC_ASSERT_MSG(
            !mgbase::is_pointer<TD>::value
        ,   "initialized with null function pointer deleter"
        );
    }
    
    explicit unique_ptr(const pointer ptr) MGBASE_NOEXCEPT
        : t_{ptr, deleter_type{}}
    {
        MGBASE_STATIC_ASSERT_MSG(
            !mgbase::is_pointer<TD>::value
        ,   "initialized with null function pointer deleter"
        );
    }

private:
    typedef typename conditional<
        mgbase::is_reference<deleter_type>::value
    ,   deleter_type
    ,   const deleter_type&
    >::type
    const_ref_deleter_type;
    
    typedef typename std::remove_reference<TD>::type&& rref_deleter_type;

public:
    unique_ptr(const pointer ptr, const_ref_deleter_type del)
        : t_{ptr, del}
        { }
    
    unique_ptr(const pointer ptr, rref_deleter_type del)
        : t_{ptr, del} { }
    
    unique_ptr(const unique_ptr&) = delete;
    
    unique_ptr(unique_ptr&& other)
        : t_{
            other.release()
        ,   std::forward<deleter_type>(other.get_deleter()) // initialize with paren
        }
        { }
    
    template <typename U, typename UD>
    unique_ptr(unique_ptr<U, UD>&& other)
        : t_{
            other.release()
        ,   std::forward<deleter_type>(other.get_deleter())
        }
        { }
    
    ~unique_ptr()
    {
        reset();
    }
    
    unique_ptr& operator = (const unique_ptr&) = delete;
    
    unique_ptr& operator = (unique_ptr&& other) MGBASE_NOEXCEPT
    {
        reset(other.release());
        get_deleter() = std::move(other.get_deleter());
        return *this;
    }
    
    template <typename U, typename UD>
    unique_ptr& operator = (unique_ptr<U, UD>&& other) MGBASE_NOEXCEPT
    {
        reset(other.release());
        get_deleter() = std::move(other.get_deleter());
        return *this;
    }
    
    void swap(unique_ptr&& other) {
        using std::swap;
        swap(t_, other.t_);
    }
    
    operator safe_bool_type() const MGBASE_NOEXCEPT {
        return get() == MGBASE_NULLPTR ? MGBASE_NULLPTR : &unique_ptr::t_;
    }
    
    void reset(const pointer ptr = MGBASE_NULLPTR)
    {
        const pointer old = get();
        std::get<0>(t_) = ptr;
        if (old != ptr) {
            get_deleter()(old);
        }
    }
    
    pointer release()
    {
        const pointer ptr = get();
        std::get<0>(t_) = MGBASE_NULLPTR;
        return ptr;
    }
    
    pointer get() const MGBASE_NOEXCEPT {
        return std::get<0>(t_);
    }
    
    deleter_type& get_deleter() MGBASE_NOEXCEPT {
        return static_cast<deleter_type&>(std::get<1>(t_));
    }
    const deleter_type& get_deleter() const MGBASE_NOEXCEPT {
        return static_cast<const deleter_type&>(std::get<1>(t_));
    }
    
    T& operator * () const MGBASE_NOEXCEPT {
        return *get();
    }
    
    pointer operator -> () const MGBASE_NOEXCEPT {
        return get();
    }
    
private:
    tuple_type t_;
};


template <typename T, typename TD>
class unique_ptr<T [], TD>
{
public:
    typedef T*  pointer;
    typedef T   element_type;
    typedef TD  deleter_type;
    
private:
    typedef std::tuple<pointer, deleter_type>   tuple_type;
    typedef tuple_type unique_ptr::*            safe_bool_type;

public:
    unique_ptr() MGBASE_NOEXCEPT
        : t_{MGBASE_NULLPTR, deleter_type{}}
    {
        MGBASE_STATIC_ASSERT_MSG(
            !mgbase::is_pointer<TD>::value
        ,   "initialized with null function pointer deleter"
        );
    }
    
    explicit unique_ptr(const pointer ptr) MGBASE_NOEXCEPT
        : t_{ptr, deleter_type{}}
    {
        MGBASE_STATIC_ASSERT_MSG(
            !mgbase::is_pointer<TD>::value
        ,   "initialized with null function pointer deleter"
        );
    }
    
private:
    typedef typename conditional<
        mgbase::is_reference<deleter_type>::value
    ,   deleter_type
    ,   const deleter_type&
    >::type
    const_ref_deleter_type;
    
    typedef typename std::remove_reference<TD>::type&& rref_deleter_type;

public:
    unique_ptr(const pointer ptr, const_ref_deleter_type del)
        : t_{ptr, del}
        { }
    
    unique_ptr(const pointer ptr, rref_deleter_type del)
        : t_{ptr, del} { }
    
    unique_ptr(const unique_ptr&) = delete;
    
    unique_ptr(unique_ptr&& other)
        : t_{
            other.release()
        ,   std::forward<deleter_type>(other.get_deleter()) // initialize with paren
        }
        { }
    
    template <typename U, typename UD>
    unique_ptr(unique_ptr<U, UD>&& other)
        : t_{
            other.release()
        ,   std::forward<deleter_type>(other.get_deleter())
        }
        { }
    
    ~unique_ptr()
    {
        reset();
    }
    
    unique_ptr& operator = (const unique_ptr&) = delete;
    
    template <typename U, typename UD>
    unique_ptr& operator = (unique_ptr<U, UD>&& other) MGBASE_NOEXCEPT
    {
        reset(other.release());
        get_deleter() = std::move(other.get_deleter());
        return *this;
    }
    
    void swap(unique_ptr&& other) {
        using std::swap;
        swap(t_, other.t_);
    }
    
    operator safe_bool_type() const MGBASE_NOEXCEPT {
        return get() == MGBASE_NULLPTR ? MGBASE_NULLPTR : &unique_ptr::t_;
    }
    
    void reset(const pointer ptr = MGBASE_NULLPTR)
    {
        const pointer old = get();
        std::get<0>(t_) = ptr;
        if (old != ptr) {
            get_deleter()(old);
        }
    }
    
    pointer release()
    {
        const pointer ptr = get();
        std::get<0>(t_) = MGBASE_NULLPTR;
        return ptr;
    }
    
    element_type& operator[] (mgbase::size_t index) const {
        MGBASE_ASSERT(get() != MGBASE_NULLPTR);
        return get()[index];
    }
    
    pointer get() const MGBASE_NOEXCEPT {
        return std::get<0>(t_);
    }
    
    deleter_type& get_deleter() MGBASE_NOEXCEPT {
        return static_cast<deleter_type&>(std::get<1>(t_));
    }
    const deleter_type& get_deleter() const MGBASE_NOEXCEPT {
        return static_cast<const deleter_type&>(std::get<1>(t_));
    }
    
    T& operator * () const MGBASE_NOEXCEPT {
        return *get();
    }
    
    pointer operator -> () const MGBASE_NOEXCEPT {
        return get();
    }
    
private:
    tuple_type t_;

};


} // namespace mgbase

#endif

namespace mgbase {

template <typename T, typename... Args>
inline unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>(
        new T(std::forward<Args>(args)...)
    );
}

} // namespace mgbase

