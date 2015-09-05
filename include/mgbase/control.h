
#pragma once

#include <mgbase/lang.h>

MGBASE_EXTERN_C_BEGIN

typedef void (*mgbase_control_handler_t)(void*);

struct mgbase_control_cb_common {
    mgbase_control_handler_t    handler;
    bool                        finished;
};

MGBASE_EXTERN_C_END

