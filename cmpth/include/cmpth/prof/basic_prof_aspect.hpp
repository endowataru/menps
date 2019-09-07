
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_prof_aspect
{
private:
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using worker_num_type = typename base_ult_itf_type::worker_num_type;
    
    using recorder_type = typename P::recorder_type;
    
public:
    using kind_type = typename recorder_type::kind_type;
    
    template <kind_type Kind>
    class scoped_event
    {
        using record_type = typename recorder_type::template record_t<Kind>;
        
    public:
        scoped_event() {
            const auto wk_num = base_ult_itf_type::get_worker_num();
            auto& rec = basic_prof_aspect::get_recorder();
            this->r_ = rec.template begin<Kind>(wk_num);
        }
        ~scoped_event() {
            const auto wk_num = base_ult_itf_type::get_worker_num();
            auto& rec = basic_prof_aspect::get_recorder();
            rec.template end<Kind>(wk_num, this->r_);
        }
        
        scoped_event(const scoped_event&) = delete;
        scoped_event& operator = (const scoped_event&) = delete;
        
    private:
        record_type r_;
    };
    
    template <kind_type Kind>
    static void add_event()
    {
        const auto wk_num = base_ult_itf_type::get_worker_num();
        auto& rec = basic_prof_aspect::get_recorder();
        rec.template add<Kind>(wk_num);
    }
    
    static void print_all(const char* const module_name, const fdn::size_t proc_id) {
        auto& rec = basic_prof_aspect::get_recorder();
        rec.print_all(module_name, proc_id);
    }

private:
    static recorder_type& get_recorder() noexcept {
        static recorder_type r;
        return r;
    }
};

} // namespace cmpth

#define CMPTH_DEFINE_PROF_ASPECT_POLICY_KIND(name)    name,
#define CMPTH_DEFINE_PROF_ASPECT_POLICY_NAME(name)    #name,

#define CMPTH_DEFINE_PROF_ASPECT_POLICY(KINDS) \
    private: \
        using size_type = ::cmpth::fdn::size_t; \
        \
    public: \
        enum class kind_type : size_type { \
            KINDS(CMPTH_DEFINE_PROF_ASPECT_POLICY_KIND) end \
        }; \
        \
        static const size_type num_kinds = \
            static_cast<size_type>(kind_type::end); \
        \
        static const char* get_kind_name(const kind_type k) {\
            static const char* const names[] = \
                { KINDS(CMPTH_DEFINE_PROF_ASPECT_POLICY_NAME) "end" }; \
            \
            return names[static_cast<size_type>(k)]; \
        }

