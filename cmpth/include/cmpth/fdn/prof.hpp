
#pragma once

#include <cmpth/fdn/fdn.hpp>

#define CMPTH_P_PROF_SCOPE(P, kind) \
    typename P::prof_aspect_type::template scoped_event< \
        P::prof_aspect_type::kind_type::kind \
    > CMPTH_PP_CAT(_prof_, __LINE__)

#define CMPTH_P_PROF_ADD(P, kind) \
    (P::prof_aspect_type::template add_event< \
        P::prof_aspect_type::kind_type::kind>()) \

