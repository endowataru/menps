
#pragma once

#include <mgcom/rpc/common.hpp>
#include <mgbase/scope/basic_unique_resource.hpp>
#include <mgcom/ult.hpp>
#include <mgbase/type_traits/is_void.hpp>

namespace mgcom {
namespace rpc {

namespace detail {

struct message_deleter
{
    void operator() (void* const ptr)
    {
        const auto p = static_cast<mgbase::uint8_t*>(ptr);
        
        delete[] p;
    }
};

template <typename T>
class message;

template <typename T>
struct message_traits
{
    typedef message<T>      derived_type;
    typedef T*              resource_type;
    typedef message_deleter deleter_type;
};

template <typename T>
class message
    : public mgbase::basic_unique_resource<detail::message_traits<T>>
{
    typedef mgbase::basic_unique_resource<detail::message_traits<T>>   base;
    
public:
    message() /*MGBASE_NOEXCEPT (TODO)*/ = default;
    
    message(T* ptr, const mgbase::size_t size) MGBASE_NOEXCEPT
        : base(mgbase::move(ptr))
        , size_(size)
    { }
    
    message(const message&) = delete;
    message& operator = (const message&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_1(message, base, size_)
    
    mgbase::size_t size_in_bytes() const MGBASE_NOEXCEPT {
        return size_;
    }
    
private:
    friend class mgbase::basic_unique_resource_access;
    
    bool is_owned() const MGBASE_NOEXCEPT {
        return this->get_resource();
    }
    
    void set_unowned() MGBASE_NOEXCEPT {
        this->get_resource() = MGBASE_NULLPTR;
    }
    void set_owned() MGBASE_NOEXCEPT {
        // do nothing
    }
    
    mgbase::size_t size_;
};

inline message<void> allocate_message(mgbase::size_t /*alignment*/, mgbase::size_t size)
{
    const auto ptr = new mgbase::uint8_t[size];
    
    return { ptr, size };
}

// if T != void
template <typename T, typename... Args>
inline
typename mgbase::enable_if<
    ! mgbase::is_void<T>::value
,   message<T>
>::type
make_message(Args&&... args)
{
    const auto size = sizeof(T);
    
    auto req = allocate_message(MGBASE_ALIGNOF(T), size);
    
    // Do placement new.
    const auto ptr =
        new (req.release()) T(mgbase::forward<Args>(args)...);
    
    return { ptr, size };
}

// if T == void
template <typename T>
inline
typename mgbase::enable_if<
    mgbase::is_void<T>::value
,   message<T>
>::type
make_message()
{
    return allocate_message(0, 0);
}

} // namespace detail

#define DEFINE_MESSAGE_TYPE(NAME) \
    template <typename T> \
    class NAME \
        : public detail::message<T> \
    { \
        typedef detail::message<T>  base; \
        \
    public: \
        NAME() /*MGBASE_NOEXCEPT (TODO)*/ = default; \
        \
        NAME(const NAME&) = delete; \
        NAME& operator = (const NAME&) = delete; \
        \
        MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(NAME, base) \
        \
        NAME(T* ptr, const mgbase::size_t size) \
            : base(ptr, size) \
        { } \
        \
        template <typename U> \
        static NAME convert_from(detail::message<U> msg) { \
            const auto size = msg.size_in_bytes(); \
            NAME r(static_cast<T*>(msg.release()), size); \
            return r; \
        } \
    };

DEFINE_MESSAGE_TYPE(server_request_message)
DEFINE_MESSAGE_TYPE(server_reply_message)
DEFINE_MESSAGE_TYPE(client_request_message)
DEFINE_MESSAGE_TYPE(client_reply_message)

#undef DEFINE_MESSAGE_TYPE

template <typename T, typename... Args>
inline client_request_message<T> make_request(Args&&... args)
{
    return client_request_message<T>::convert_from(
        detail::make_message(mgbase::forward<Args>(args)...)
    );
}

} // namespace rpc
} // namespace mgcom

