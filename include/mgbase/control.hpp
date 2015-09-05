
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/control.h>
#include <mgbase/assert.hpp>

namespace mgbase {

namespace control {

typedef ::mgbase_control_handler_t  handler_t;
typedef ::mgbase_control_cb_common  cb_common;

template <typename T, void (*func)(T&)>
struct handling {
    static void f(void* ptr) {
        func(*static_cast<T*>(ptr));
    }
};

namespace {

template <typename CB>
inline void finished(CB& cb){
    cb.common.finished = true;
}


template <typename CB>
inline void enter(CB& cb, handler_t handler) {
    cb.common.handler = handler;
    handler(&cb);
}

template <typename CB, void (*func)(CB&)>
inline void enter(CB& cb) {
    enter(cb, &handling<CB, func>::f);
}

template <typename CB, void (*func)(CB&)>
inline void start(CB& cb) {
    cb.common.finished = false;
    enter<CB, func>(cb);
}

template <typename CB>
inline void dispatch(CB& cb) {
    MGBASE_ASSERT(cb.common.handler != MGBASE_NULLPTR);
    cb.common.handler(&cb);
}

template <typename T>
inline bool test(T& cb) MGBASE_NOEXCEPT {
    return cb.common.finished;
}

template <typename T>
inline bool proceed(T& cb) {
    if (test(cb))
        return true;
    else {
        dispatch(cb);
        return false;
    }
}

template <typename CB>
inline void wait(CB& cb) {
    while (!test(cb))
        dispatch(cb);
}

}

}

}

