
#pragma once

#include <cmpth/prof/basic_prof_aspect.hpp>
#include <cmpth/prof/prof_tag.hpp>

namespace cmpth {

template <typename P>
class dummy_prof_recorder
{
private:
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using worker_num_type = typename base_ult_itf_type::worker_num_type;
    
public:
    using kind_type = typename P::kind_type;
    
    struct record_type { };
    
    template <kind_type Kind>
    using record_t = record_type;

    void set_enabled(const bool /*is_enabled*/) { }
    
    template <kind_type Kind>
    record_type begin() { return {}; }
    
    template <kind_type Kind>
    void end(record_type /*r*/) { }
    
    template <kind_type Kind>
    void add() { }

    void print_all(const char* /*module_name*/, fdn::size_t /*proc_id*/) { }
};

template <typename UltItf, typename P2>
struct dummy_prof_aspect_policy
{
    using base_ult_itf_type = UltItf;
    using recorder_type = dummy_prof_recorder<dummy_prof_aspect_policy>;
    using kind_type = typename P2::kind_type;
};

template <typename UltItf, typename P2>
struct get_prof_aspect_type<prof_tag::DUMMY, UltItf, P2>
    : fdn::type_identity<
        basic_prof_aspect<dummy_prof_aspect_policy<UltItf, P2>>
    > { };

} // namespace cmpth

