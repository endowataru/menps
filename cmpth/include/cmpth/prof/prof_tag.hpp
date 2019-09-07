
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

enum class prof_tag
{
    DUMMY = 1
,   STAT
,   TRACE
};

template <prof_tag ProfTag, typename UltItf, typename P2>
struct get_prof_aspect_type;

template <prof_tag ProfTag, typename UltItf, typename P2>
using get_prof_aspect_t = typename get_prof_aspect_type<ProfTag, UltItf, P2>::type;

#define CMPTH_PROF_HEADER_DUMMY <cmpth/prof/dummy_prof_recorder.hpp>
#define CMPTH_PROF_HEADER_STAT  <cmpth/prof/stat_prof_aspect.hpp>
#define CMPTH_PROF_HEADER_TRACE <cmpth/prof/trace_prof_aspect.hpp>

} // namespace cmpth

