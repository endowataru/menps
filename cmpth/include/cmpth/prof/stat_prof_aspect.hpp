
#pragma once

#include <cmpth/prof/basic_prof_aspect.hpp>
#include <cmpth/prof/stat_prof_recorder.hpp>
#include <cmpth/prof/stat_accumulator.hpp>
#include <cmpth/prof/prof_tag.hpp>
#include <cmpth/arch/x86_64_cpu_clock_policy.hpp>

namespace cmpth {

template <typename UltItf, typename P2>
struct stat_prof_accumulator_policy
{
    using clock_policy_type = x86_64_cpu_clock_policy;
    using sample_type = clock_policy_type::clock_type;
    using real_type = double;
};

template <typename UltItf, typename P2>
struct stat_prof_recorder_policy
{
private:
    using accumulator_policy_type = stat_prof_accumulator_policy<UltItf, P2>;
    
public:
    using base_ult_itf_type = UltItf;
    using clock_policy_type =
        typename accumulator_policy_type::clock_policy_type;
    
    using accumulator_type = stat_accumulator<accumulator_policy_type>;
    using assert_aspect_type = typename UltItf::assert_aspect;
    
    using kind_type = typename P2::kind_type;
    static const fdn::size_t num_kinds = P2::num_kinds;
    static const char* get_kind_name(const kind_type kind) noexcept {
        return P2::get_kind_name(kind);
    }
};

template <typename UltItf, typename P2>
struct stat_prof_aspect_policy
{
    using base_ult_itf_type = UltItf;
    using recorder_type = stat_prof_recorder<stat_prof_recorder_policy<UltItf, P2>>;
};

template <typename UltItf, typename P2>
struct get_prof_aspect_type<prof_tag::STAT, UltItf, P2>
    : fdn::type_identity<
        basic_prof_aspect<stat_prof_aspect_policy<UltItf, P2>>
    > { };

} // namespace cmpth
