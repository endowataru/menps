
#pragma once

#include <menps/mecom/rpc/common.hpp>
#include <menps/mefdn/basic_dereferencable.hpp>
#include <menps/mefdn/scope/basic_unique_resource.hpp>
#include <menps/mecom/ult.hpp>
#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mecom {
namespace rpc {

namespace detail {

struct message_deleter
{
    void operator() (void* const ptr)
    {
        const auto p = static_cast<mefdn::uint8_t*>(ptr);
        
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
    : public mefdn::basic_unique_resource<detail::message_traits<T>>
    , public mefdn::basic_dereferencable<detail::message_traits<T>>
{
    typedef mefdn::basic_unique_resource<detail::message_traits<T>>   base;
    
public:
    message() /*noexcept (TODO)*/ = default;
    
    message(T* ptr, const mefdn::size_t size) noexcept
        : base(mefdn::move(ptr))
        , size_(size)
    { }
    
    message(const message&) = delete;
    message& operator = (const message&) = delete;
    
    message(message&&) /*noexcept*/ = default; // TODO: exception specification does not match
    message& operator = (message&&) /*noexcept*/ = default;
    
    mefdn::size_t size_in_bytes() const noexcept {
        return size_;
    }
    
private:
    friend class mefdn::basic_unique_resource_access;
    
    bool is_owned() const noexcept {
        return this->get_resource();
    }
    
    void set_unowned() noexcept {
        this->get_resource() = nullptr;
    }
    void set_owned() noexcept {
        // do nothing
    }
    
    mefdn::size_t size_;
};

inline message<void> allocate_message(mefdn::size_t /*alignment*/, mefdn::size_t size)
{
    const auto ptr = new mefdn::uint8_t[size];
    
    return { ptr, size };
}

// if T != void
template <typename T, typename... Args>
inline
typename mefdn::enable_if<
    ! mefdn::is_void<T>::value
,   message<T>
>::type
make_message(Args&&... args)
{
    const auto size = sizeof(T);
    
    auto req = allocate_message(alignof(T), size);
    
    // Do placement new.
    const auto ptr =
        new (req.release()) T(mefdn::forward<Args>(args)...);
    
    return { ptr, size };
}

// if T == void
template <typename T>
inline
typename mefdn::enable_if<
    mefdn::is_void<T>::value
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
        NAME() /*noexcept (TODO)*/ = default; \
        \
        NAME(const NAME&) = delete; \
        NAME& operator = (const NAME&) = delete; \
        \
        NAME(NAME&&) /*noexcept (TODO)*/ = default; \
        NAME& operator = (NAME&&) /*noexcept (TODO)*/ = default; \
        \
        NAME(T* ptr, const mefdn::size_t size) \
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
        detail::make_message(mefdn::forward<Args>(args)...)
    );
}

} // namespace rpc
} // namespace mecom
} // namespace menps

