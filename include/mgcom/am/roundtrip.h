
#pragma once

#include <mgcom/common.h>
#include <mgcom/am/untyped.h>

MGBASE_EXTERN_C_BEGIN

typedef struct mgcom_am_call_roundtrip_cb
{
    MGBASE_CONTINUATION(void)   cont;
    bool                        got_reply;
    mgcom_am_send_cb            cb_send;
}
mgcom_am_call_roundtrip_cb;


MGBASE_EXTERN_C_END

