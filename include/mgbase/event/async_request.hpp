
#pragma once

#include <mgbase/lang.hpp>
#include "async_request.h"

namespace mgbase {

struct async_request_header {
    bool (*func_test)(mgbase_async_request*);
    size_t state;
    size_t required_size;
    void*  dynamic_buffer;
};

typedef ::mgbase_async_request  async_request;

inline bool test(async_request& request) {
    if (request.state == MGBASE_ASYNC_STATE_FINISHED)
        return true;
    else {
        request.func_test(&request);
        return false;
    }
}

#define MGBASE_SCOPED_ASYNC(name, func, ...) \
    mgbase::async_request name; \
    (func)(&name, __VA_ARGS__); \
    name.dynamic_buffer = alloca(name.required_size);

}

