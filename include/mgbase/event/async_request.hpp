
#pragma once

#include <mgbase/lang.hpp>
#include "async_request.h"

namespace mgbase {

struct async_request_header {
    bool (*func_test)(mgbase_async_request*);
    size_t state;
    size_t required_size;
};

typedef ::mgbase_async_request  async_request;

inline bool test(async_request& request) {
    if (request.state == MGBASE_STATE_FINISHED)
        return true;
    else {
        request.func_test(&request);
        return false;
    }
}

}

