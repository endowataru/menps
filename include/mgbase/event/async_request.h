
#pragma once

#include <mgbase/lang.h>
#include <alloca.h>

MGBASE_EXTERN_C_BEGIN

#define MGBASE_STATE_FINISHED        (size_t)-1

struct mgbase_async_request {
    bool (*func_test)(mgbase_async_request*);
    size_t state;
    size_t required_size;
    void*       dynamic_buffer;
    char        static_buffer[128];
};

MGBASE_EXTERN_C_END

