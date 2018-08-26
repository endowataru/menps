
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/medsm2/config.h>

#define MEDSM2_ENABLE_P2P_LOCK
#define MEDSM2_ENABLE_NEEDS_LOCAL_COMP

//#define MEOMP_SEPARATE_WORKER_THREAD
    // TODO: Rename

namespace menps {
namespace medsm2 {

using default_ult_itf = mecom2::default_ult_itf;

} // namespace medsm2
} // namespace menps
