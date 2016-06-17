
#pragma once

#include <mgcom/common.hpp>
#include <mgcom/rpc/call.h>
#include <mgbase/optional.hpp>
#include <mgbase/operation.hpp>

namespace mgcom {
namespace rpc {

struct constants {
    static const index_t max_num_handlers = MGCOM_RPC_MAX_NUM_HANDLERS;
};

typedef mgcom_rpc_handler_parameters    handler_parameters;
typedef mgcom_rpc_handler_function_t    handler_function_t;
typedef mgcom_rpc_handler_id_t          handler_id_t;

namespace untyped {

struct register_handler_params
{
    handler_id_t            id;
    handler_function_t      callback;
};

struct call_params
{
    process_id_t        proc;
    handler_id_t        handler_id;
    const void*         arg_ptr;
    index_t             arg_size;
    void*               return_ptr;
    index_t             return_size;
    mgbase::operation   on_complete;
};

} // namespace untyped

class requester
    : mgbase::noncopyable
{
public:
    virtual ~requester() MGBASE_EMPTY_DEFINITION
    
    virtual void register_handler(const untyped::register_handler_params& params) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_call_async(const untyped::call_params& params) = 0;
    
    static requester& get_instance() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(req_ != MGBASE_NULLPTR);
        return *req_;
    }
    
    static void set_instance(requester& req) {
        req_ = &req;
    }
    
private:
    static requester* req_;
};


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

namespace untyped {

inline void register_handler(
    const handler_id_t          id
,   const handler_function_t    callback
) {
    register_handler_params params = { id, callback };
    requester::get_instance().register_handler(params);
}

MGBASE_WARN_UNUSED_RESULT
inline bool try_remote_call_async(
    const process_id_t          proc
,   const handler_id_t          handler_id
,   const void* const           arg_ptr
,   const index_t               arg_size
,   void* const                 return_ptr
,   const index_t               return_size
,   const mgbase::operation&    on_complete
) {
    call_params params = { proc, handler_id, arg_ptr,
        arg_size, return_ptr, return_size, on_complete };
    
    return requester::get_instance().try_call_async(params);
}

} // namespace untyped

namespace /*unnamed*/ {

template <typename Handler>
MGBASE_WARN_UNUSED_RESULT
inline bool try_remote_call_async(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   typename Handler::return_type* const    return_result
,   const mgbase::operation&                on_complete
) {
    typedef detail::handler_types<Handler>  types;
    typedef typename types::argument_info   argument_info;
    typedef typename Handler::return_type   return_type;
    
    const argument_info arg_info = mgbase::wrap_value(arg);
    
    return untyped::try_remote_call_async(
        proc
    ,   Handler::handler_id
    ,   &arg_info
    ,   sizeof(argument_info)
    ,   return_result
    ,   sizeof(return_type)
    ,   on_complete
    );
}

template <typename Handler>
MGBASE_WARN_UNUSED_RESULT
inline bool try_remote_call_async(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   const mgbase::operation&                on_complete
) {
    typedef detail::handler_types<Handler>  types;
    typedef typename types::argument_info   argument_info;
    typedef typename Handler::return_type   return_type;
    
    MGBASE_STATIC_ASSERT_MSG(
        mgbase::is_void<return_type>::value
    ,   "Handler::return_type must be void if this overload is chosen"
    );
    
    const argument_info arg_info = mgbase::wrap_value(arg);
    
    return untyped::try_remote_call_async(
        proc
    ,   Handler::handler_id
    ,   &arg_info
    ,   sizeof(argument_info)
    ,   MGBASE_NULLPTR
    ,   0
    ,   on_complete
    );
}

} // unnamed namespace

namespace detail {

template <typename Handler>
inline index_t on_callback(const handler_parameters* params)
{
    typedef detail::handler_types<Handler>  types;
    typedef typename types::argument_info   argument_info;
    typedef typename Handler::return_type   return_type;
    
    const argument_info& arg_info = *static_cast<const argument_info*>(params->data);
    
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

} // namespace detail

template <typename Handler>
inline void register_handler()
{
    untyped::register_handler(
        Handler::handler_id
    ,   &detail::on_callback<Handler>
    );
}

} // namespace rpc
} // namespace mgcom

