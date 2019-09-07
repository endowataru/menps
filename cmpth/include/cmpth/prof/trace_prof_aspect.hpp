
#pragma once

#include <cmpth/prof/basic_prof_aspect.hpp>
#include <cmpth/prof/trace_prof_recorder.hpp>
#include <cmpth/prof/prof_tag.hpp>
#include <cmpth/arch/x86_64_cpu_clock_policy.hpp>

namespace cmpth {

template <typename UltItf, typename P2>
struct trace_prof_recorder_policy
{
    using base_ult_itf_type = UltItf;
    using kind_type = typename P2::kind_type;
    using clock_policy_type = x86_64_cpu_clock_policy;
    
    static constexpr fdn::size_t get_default_mlog_size() noexcept {
        return P2::get_default_mlog_size();
    }
    static const char* get_kind_name(const kind_type kind) noexcept {
        return P2::get_kind_name(kind);
    }
};

template <typename UltItf, typename P2>
struct trace_prof_aspect_policy
{
    using base_ult_itf_type = UltItf;
    using recorder_type =
        trace_prof_recorder<trace_prof_recorder_policy<UltItf, P2>>;
};

template <typename UltItf, typename P2>
struct get_prof_aspect_type<prof_tag::TRACE, UltItf, P2>
    : fdn::type_identity<
        basic_prof_aspect<trace_prof_aspect_policy<UltItf, P2>>
    > { };

} // namespace cmpth

