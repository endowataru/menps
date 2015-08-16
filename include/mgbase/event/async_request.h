
#pragma once

#include <mgbase/lang.h>

MGBASE_EXTERN_C_BEGIN

#define MGBASE_STATE_FINISHED        static_cast<mgbase::size_t>(-1)

struct mgbase_async_request {
    bool (*func_test)(mgbase_async_request*);
    size_t state;
    size_t required_size;
};

MGBASE_EXTERN_C_END

