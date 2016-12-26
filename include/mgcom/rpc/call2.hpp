
#pragma once

#include <mgcom/rpc/requester.hpp>
#include <mgbase/scope/basic_unique_resource.hpp>
#include <mgcom/ult.hpp>
#include <mgbase/type_traits/is_void.hpp>

namespace mgcom {
namespace rpc {

template <typename T>
class request_message;

template <typename T>
class reply_message;

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

} // namespace detail

template <typename T>
class request_message
    : public detail::message<T>
{
    typedef detail::message<T>  base;
    
public:
    request_message() /*MGBASE_NOEXCEPT (TODO)*/ = default;
    
    request_message(T* ptr, const mgbase::size_t size)
        : base(ptr, size)
    { }
};

template <typename T>
class reply_message
    : public detail::message<T>
{
    typedef detail::message<T>  base;
    
public:
    reply_message() /*MGBASE_NOEXCEPT (TODO)*/ = default;
    
    reply_message(T* ptr, const mgbase::size_t size)
        : base(ptr, size)
    { }
};

inline request_message<void> allocate_request(mgbase::size_t /*alignment*/, mgbase::size_t size)
{
    const auto ptr = new mgbase::uint8_t[size];
    
    return { ptr, size };
}

// if T != void
template <typename T, typename... Args>
inline
typename mgbase::enable_if<
    ! mgbase::is_void<T>::value
,   request_message<T>
>::type
make_request(Args&&... args)
{
    const auto size = sizeof(T);
    
    auto req = allocate_request(MGBASE_ALIGNOF(T), size);
    
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
,   request_message<T>
>::type
make_request()
{
    return allocate_request(0, 0);
}

/*
    struct example_fixed_sized_handler
    {
        static const mgcom::rpc::handler_id_t handler_id = ???;
        
        typedef double  request_type;
        typedef int     reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            auto& req = sc.request();
            const auto size = sc.size_in_bytes();
            const auto src_proc = sc.src_proc();
            
            // ...
            
            
            auto ret = server.make_reply();
            // *ret = ...;
            return ret;
        }
    };
    
    struct example_variable_length_handler
    {
        static const mgcom::rpc::handler_id_t handler_id = ???;
        
        typedef double  request_type;
        typedef int     reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            auto& req = sc.request();
            const auto size = sc.size_in_bytes();
            const auto src_proc = sc.src_proc();
            
            // ...
            
            
            auto ret =
                sc.allocate_reply(
                    MGBASE_ALIGNOF(reply_type)
                ,   sizeof(reply_type) + // ...
                );
            
            // *ret = ...;
            
            return ret;
        }
    };
*/

namespace detail {

template <typename Handler>
class server_context
{
    typedef typename Handler::request_type  request_type;
    typedef typename Handler::reply_type    reply_type;
    
public:
    typedef request_message<reply_type>     return_type;
    
    server_context(
        const process_id_t          src_proc
    ,   const request_type* const   data
    ,   const mgbase::size_t        size
    )
        : src_proc_(src_proc)
        , data_(data)
        , size_(size)
    { }
    
    // getter functions
    
    const request_type& request() const MGBASE_NOEXCEPT {
        return *data_;
    }
    const request_type* request_ptr() const MGBASE_NOEXCEPT {
        return data_;
    }
    
    mgbase::size_t size_in_bytes() const MGBASE_NOEXCEPT {
        return size_;
    }
    
    process_id_t src_proc() const MGBASE_NOEXCEPT {
        return src_proc_;
    }
    
    // helper functions
    
    template <typename... Args>
    return_type make_reply(Args&&... args) const {
        return make_request<reply_type>(mgbase::forward<Args>(args)...);
    }
    
private:
    process_id_t        src_proc_;
    const request_type* data_;
    mgbase::size_t      size_;
};

template <typename Handler>
inline index_t handler_pass(void* const ptr, const handler_parameters* const params)
{
    typedef typename Handler::request_type request_type;
    
    const server_context<Handler> p{
        params->source
    ,   static_cast<const request_type*>(params->data)
    ,   params->size
    };
    
    auto& handler = *static_cast<Handler*>(ptr);
    
    auto ret_msg = handler(p);
    
    const auto size = ret_msg.size_in_bytes();
    
    memcpy(params->result, ret_msg.get(), size);
    
    return size;
}

} // namespace detail

template <typename Handler>
void register_handler2(requester& req, Handler h)
{
    // TODO
    static Handler handler(mgbase::move(h));
    
    req.register_handler({
        Handler::handler_id
    ,   & detail::handler_pass<Handler>
    ,   &handler
    });
}


template <typename Handler>
inline void call2_async(
    requester&                                              req
,   const process_id_t                                      target_proc
,   const request_message<typename Handler::request_type>   rqst_msg
,   reply_message<typename Handler::reply_type>* const      rply_out
,   const mgbase::callback<void ()>                         on_complete
) {
    typedef typename Handler::reply_type   reply_type;
    
    reply_message<reply_type> rply_msg(
        reinterpret_cast<reply_type*>(
            new mgbase::uint8_t[MGCOM_RPC_MAX_DATA_SIZE]
        )
    ,   MGCOM_RPC_MAX_DATA_SIZE
    );
    
    const untyped::call_params params{
        target_proc
    ,   Handler::handler_id
    ,   rqst_msg.get()
    ,   rqst_msg.size_in_bytes()
    ,   rply_msg.get()
    ,   rply_msg.size_in_bytes()
    ,   on_complete
    };
    
    while (!req.try_call_async(params)) {
        ult::this_thread::yield();
    }
    
    *rply_out = mgbase::move(rply_msg);
}

template <typename Handler>
inline reply_message<typename Handler::reply_type> call2(
    requester&                                      rqstr
,   const process_id_t                              target_proc
,   request_message<typename Handler::request_type> rqst_msg
) {
    typedef typename Handler::reply_type   reply_type;
    
    reply_message<reply_type> rply_msg;
    
    mgbase::atomic<bool> flag{false};
    
    call2_async<Handler>(
        rqstr
    ,   target_proc
    ,   mgbase::move(rqst_msg)
    ,   &rply_msg
    ,   mgbase::make_callback_store_release(&flag, MGBASE_NONTYPE(true))
    );
    
    while (!flag.load(mgbase::memory_order_acquire)) {
        ult::this_thread::yield();
    }
    
    return rply_msg;
}

template <typename Handler>
inline reply_message<typename Handler::reply_type> call2(
    requester&                              rqstr
,   const process_id_t                      target_proc
,   const typename Handler::request_type&   rqst_data
) {
    return call2<Handler>(
        rqstr
    ,   target_proc
    ,   make_request<typename Handler::request_type>(rqst_data)
    );
}

} // namespace rpc
} // namespace mgcom

