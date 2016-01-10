
#pragma once

#include <mgcom/am.h>

namespace mgcom {
namespace am {

typedef mgcom_am_handler_id_t           handler_id_t;

typedef mgcom_am_handler_callback_t     handler_callback_t;

typedef mgcom_am_callback_parameters    callback_parameters;

typedef mgcom_am_am_message_buffer      am_message_buffer;

typedef mgcom_am_send_cb  send_cb;

void poll();

namespace untyped {

namespace detail {

mgbase::deferred<void> send_nb(send_cb& cb);

} // namespace detail

namespace /*unnamed*/ {

inline mgbase::deferred<void> send_nb(
    send_cb&        cb
,   handler_id_t    id
,   const void*     value
,   index_t         size
,   process_id_t    dest_proc
) {
    cb.dest_proc = dest_proc;
    cb.msg.id    = id;
    cb.msg.size  = size;
    
    std::memcpy(cb.msg.data, value, size);
    
    return detail::send_nb(cb);
}

} // unnamed namespace

void reply(
    const callback_parameters* params
,   handler_id_t               id
,   const void*                value
,   index_t                    size
);

void register_handler(
    handler_id_t       id
,   handler_callback_t callback
);

} // namespace untyped

} // namespace am
} // namespace mgcom

