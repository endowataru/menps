
#pragma once

#include <mgbase/lang.h>
#include <mgbase/assert.hpp>

MGBASE_EXTERN_C_BEGIN

typedef void (*mgbase_control_handler_t)(void*);

typedef struct mgbase_control_cb_common {
    mgbase_control_handler_t    handler;
    bool                        finished;
}
mgbase_control_cb_common;

static inline void mgbase_control_initialize(mgbase_control_cb_common* common) MGBASE_NOEXCEPT {
    common->finished = false;
}

static inline void mgbase_control_set_finished(mgbase_control_cb_common* common) MGBASE_NOEXCEPT {
    common->finished = true;
}

static inline void mgbase_control_set_next(mgbase_control_cb_common* common, mgbase_control_handler_t handler) MGBASE_NOEXCEPT {
    MGBASE_ASSERT(!common->finished);
    common->handler = handler;
}

static inline void mgbase_control_dispatch(mgbase_control_cb_common* common) MGBASE_NOEXCEPT {
    MGBASE_ASSERT(!common->finished);
    MGBASE_ASSERT(common->handler != MGBASE_NULLPTR);
    common->handler(common);
}

static inline void mgbase_control_enter(mgbase_control_cb_common* common, mgbase_control_handler_t handler) MGBASE_NOEXCEPT {
    mgbase_control_set_next(common, handler);
    mgbase_control_dispatch(common);
}

MGBASE_EXTERN_C_END

