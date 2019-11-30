
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/medsm2/config.h>

#if !defined(MEDSM2_ENABLE_MIGRATION) && defined(MEDSM2_ENABLE_LAZY_MERGE)
    #error "Using MEDSM2_LAZY_MERGE without MEDSM2_ENABLE_MIGRATION is prohibited"
#endif

#if !defined(MEDSM2_ENABLE_LAZY_MERGE) && defined(MEDSM2_ENABLE_FAST_RELEASE)
    #error "Using MEDSM2_ENABLE_FAST_RELEASE without MEDSM2_ENABLE_LAZY_MERGE is prohibited"
#endif

#if defined(MEDSM2_ENABLE_LAZY_MERGE) && defined(MEDSM2_USE_DIRECTORY_COHERENCE)
    #error "MEDSM2_ENABLE_LAZY_MERGE cannot be enabled when MEDSM2_USE_DIRECTORY_COHERENCE is ON"
#endif


