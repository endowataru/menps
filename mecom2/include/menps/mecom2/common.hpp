
#pragma once

#include <menps/medev2/common.hpp>
#include <menps/mecom2/config.h>

//#define MECOM2_USE_QDLOCK

#ifdef MECOM2_USE_QDLOCK
    #include <menps/meult/qd/qdlock_mutex.hpp>
#endif

