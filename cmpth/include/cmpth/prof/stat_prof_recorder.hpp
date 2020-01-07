
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
struct stat_prof_recorder
{
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using worker_num_type = typename base_ult_itf_type::worker_num_type;
    
    using accumulator_type = typename P::accumulator_type;
    using clock_policy_type = typename P::clock_policy_type;
    using clock_type = typename clock_policy_type::clock_type;
    
    using accumulator_array_type = fdn::array<accumulator_type, P::num_kinds>;
    
public:
    using kind_type = typename P::kind_type;
    
    stat_prof_recorder()
        : accs_{
            fdn::make_unique<accumulator_array_type []>(
                base_ult_itf_type::get_num_workers()
            )
        }
    { }
    
    struct record_type {
        clock_type  t0;
    };

    template <kind_type Kind>
    using record_t = record_type;

    void set_enabled(const bool is_enabled) {
        this->is_enabled_ = is_enabled;
    }
    
    template <kind_type Kind>
    record_type begin(worker_num_type /*wk_num*/) {
        if (this->is_enabled_) {
            return { clock_policy_type::get_clock() };
        }
        else {
            return record_type();
        }
    }
    template <kind_type Kind>
    void end(const worker_num_type wk_num, record_type r) {
        if (!this->is_enabled_) {
            return;
        }
        const auto t1 = clock_policy_type::get_clock();
        auto& acc = this->get_accumulator(Kind, wk_num);
        acc.add(t1 - r.t0);
    }
    template <kind_type Kind>
    void add(const worker_num_type wk_num) {
        auto& acc = this->get_accumulator(Kind, wk_num);
        acc.add(0);
    }
    
    void print_all(const char* const module_name, const fdn::size_t proc_id)
    {
        std::cout << "- module: " << module_name << std::endl;
        std::cout << "  proc_id: " << proc_id << std::endl;
        this->print_all(std::cout, "  ");
    }
    template <typename OutputStream>
    void print_all(OutputStream& os, const char* const indent_str)
    {
        const auto num_wks = base_ult_itf_type::get_num_workers();
        for (fdn::size_t kind_i = 0; kind_i < P::num_kinds; ++kind_i) {
            const auto kind = static_cast<kind_type>(kind_i);
            os << indent_str << "- " << P::get_kind_name(kind) << ":" << std::endl;
            for (worker_num_type wk_num = 0; wk_num < num_wks; ++wk_num) {
                auto& acc = this->get_accumulator(kind, wk_num);
                os << indent_str << "    - ";
                acc.print_yaml(os);
                os << std::endl;
            }
        }
    }
    
private:
    accumulator_type& get_accumulator(const kind_type kind, const worker_num_type wk_num) noexcept {
        CMPTH_P_ASSERT(P, wk_num < base_ult_itf_type::get_num_workers());
        return this->accs_[wk_num][static_cast<fdn::size_t>(kind)];
    }
    
    bool is_enabled_ = true;
    fdn::unique_ptr<accumulator_array_type []> accs_;
};

} // namespace cmpth

