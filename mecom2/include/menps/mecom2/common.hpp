
#pragma once

#include <menps/medev2/common.hpp>
#include <menps/mecom2/config.h>

#ifdef MECOM2_USE_QDLOCK
    #include <menps/meult/qd/qdlock_mutex.hpp>
#endif

#if defined(MEDEV2_AVOID_SWITCH_IN_SIGNAL) && !defined(MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS)
    #error "Enable MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS for MEDEV2_AVOID_SWITCH_IN_SIGNAL"
#endif

