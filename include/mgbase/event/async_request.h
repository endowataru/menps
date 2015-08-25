
#pragma once

#include <mgbase/lang.h>

MGBASE_EXTERN_C_BEGIN

#define MGBASE_STATE_FINISHED        static_cast<uint32_t>(-1)

struct mgbase_async_request {
    void (*func_test)(void*);
    uint32_t state;
};

MGBASE_EXTERN_C_END

