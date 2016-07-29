
#pragma once

#include "requester.hpp"

namespace mgcom {
namespace rpc {

/*

struct example_handler {
    static const mgcom::rpc::handler_id_t handler_id;
    
    typedef int     argument_type;
    typedef double  return_type;
    
    static return_type on_callback(const mgcom::rpc::handler_paramters& params, const argument_type& arg) {
        
    }
};

*/

namespace detail {

template <typename T>
struct return_type_size : mgbase::integral_constant<index_t, sizeof(T)> { };

template <>
struct return_type_size<void> : mgbase::integral_constant<index_t, 0> { };

template <typename Handler>
struct handler_types
{
    typedef typename Handler::argument_type         argument_type;
    typedef typename Handler::return_type           return_type;
    
    typedef mgbase::value_wrapper<argument_type>    argument_info;
    
    static const index_t return_size = return_type_size<return_type>::value;
};

} // namespace detail

template <typename Handler>
MGBASE_WARN_UNUSED_RESULT
inline bool try_remote_call_async(
    requester&                              req
,   const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   typename Handler::return_type* const    return_result
,   const mgbase::callback<void ()>&        on_complete
) {
    typedef detail::handler_types<Handler>  types;
    typedef typename types::argument_info   argument_info;
    typedef typename Handler::return_type   return_type;
    
    const argument_info arg_info = mgbase::wrap_value(arg);
    
    return req.try_call_async({
        proc
    ,   Handler::handler_id
    ,   &arg_info
    ,   sizeof(argument_info)
    ,   return_result
    ,   sizeof(return_type)
    ,   on_complete
    });
}

template <typename Handler>
MGBASE_WARN_UNUSED_RESULT
inline bool try_remote_call_async(
    requester&                              req
,   const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   const mgbase::callback<void ()>&        on_complete
) {
    typedef detail::handler_types<Handler>  types;
    typedef typename types::argument_info   argument_info;
    typedef typename Handler::return_type   return_type;
    
    MGBASE_STATIC_ASSERT_MSG(
        mgbase::is_void<return_type>::value
    ,   "Handler::return_type must be void if this overload is chosen"
    );
    
    const argument_info arg_info = mgbase::wrap_value(arg);
    
    return req.try_call_async({
        proc
    ,   Handler::handler_id
    ,   &arg_info
    ,   sizeof(argument_info)
    ,   MGBASE_NULLPTR
    ,   0
    ,   on_complete
    });
}

template <typename Handler>
MGBASE_WARN_UNUSED_RESULT
inline bool try_remote_call_async(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   typename Handler::return_type* const    return_result
,   const mgbase::callback<void ()>&        on_complete
) {
    return try_remote_call_async<Handler>(
        requester::get_instance()
    ,   proc
    ,   arg
    ,   return_result
    ,   on_complete
    );
}

template <typename Handler>
MGBASE_WARN_UNUSED_RESULT
inline bool try_remote_call_async(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   const mgbase::callback<void ()>&        on_complete
) {
    return try_remote_call_async<Handler>(
        requester::get_instance()
    ,   proc
    ,   arg
    ,   on_complete
    );
}


namespace detail {

template <typename Handler>
inline index_t on_callback(void* /*ptr*/, const handler_parameters* params)
{
    typedef detail::handler_types<Handler>  types;
    typedef typename types::argument_info   argument_info;
    typedef typename Handler::return_type   return_type;
    
    const auto& arg_info = *static_cast<const argument_info*>(params->data);
    
    // Call the user-defined callback function.
    const mgbase::value_wrapper<return_type> res
        = mgbase::call_with_value_wrapper_2<return_type>(
            MGBASE_MAKE_INLINED_FUNCTION_TEMPLATE(&Handler::on_request)
        ,   mgbase::wrap_reference(*params)
        ,   arg_info
        );
    
    return_type* const ret = static_cast<return_type*>(params->result);
    
    // Set the result.
    res.assign_to(ret);
    
    return return_type_size<return_type>::value;
}

template <typename Handler, typename T>
inline index_t on_callback_ptr(void* ptr, const handler_parameters* params)
{
    typedef detail::handler_types<Handler>  types;
    typedef typename types::argument_info   argument_info;
    typedef typename Handler::return_type   return_type;
    
    const auto& arg_info = *static_cast<const argument_info*>(params->data);
    
    // Call the user-defined callback function.
    const auto res = mgbase::call_with_value_wrapper_(
        arg_info
    ,   &Handler::on_request
    ,   *static_cast<T*>(ptr)
    ,   *params
    );
    
    const auto ret = static_cast<return_type*>(params->result);
    
    // Set the result.
    res.assign_to(ret);
    
    return return_type_size<return_type>::value;
}

} // namespace detail


template <typename Handler>
inline void register_handler(requester& req)
{
    req.register_handler({
        Handler::handler_id
    ,   &detail::on_callback<Handler>
    ,   MGBASE_NULLPTR
    });
}

template <typename Handler, typename T>
inline void register_handler(requester& req, T& val)
{
    req.register_handler({
        Handler::handler_id
    ,   &detail::on_callback_ptr<Handler, T>
    ,   &val
    });
}

template <typename Handler>
inline void register_handler()
{
    register_handler<Handler>(requester::get_instance());
}

} // namespace rpc
} // namespace mgcom

