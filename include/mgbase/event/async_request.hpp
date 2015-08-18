
#pragma once

#include <mgbase/lang.hpp>
#include "async_request.h"

namespace mgbase {

typedef ::mgbase_async_request  async_request;

namespace {


template <typename CB>
inline bool async_test(CB& cb) {
    if (cb.request.state == MGBASE_STATE_FINISHED)
        return true;
    else {
        cb.request.func_test(&cb.request);
        return false;
    }
}

template <typename T>
inline void async_enter(T* self, void (*func)(void*)) {
    self->request.func_test = func;
    func(self);
}

template <typename T>
inline void async_finished(T* self) {
    self->request.state = MGBASE_STATE_FINISHED;
}

}

}

