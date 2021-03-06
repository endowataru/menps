
#pragma once

#include <cmpth/fdn.hpp>
#include <mlog/mlog.h>
#include <cinttypes>

namespace cmpth {

template <typename P>
class trace_prof_recorder
{
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using worker_num_type = typename base_ult_itf_type::worker_num_type;
    
    using clock_policy_type = typename P::clock_policy_type;
    using clock_type = typename clock_policy_type::clock_type;
    
public:
    using kind_type = typename P::kind_type;
    
    struct record_type {
        void* begin_ptr;
    };
    
    template <kind_type Kind>
    using record_t = record_type;
    
    trace_prof_recorder() {
        const auto num_wks = base_ult_itf_type::get_num_workers();
        const auto num_ranks = static_cast<int>(num_wks);
        mlog_init(&this->md_, num_ranks, P::get_default_mlog_size());
    }
    
    void set_enabled(const bool is_enabled) {
        this->is_enabled_ = is_enabled;
    }
    
    template <kind_type Kind>
    record_type begin()
    {
        if (!this->is_enabled_) {
            return { nullptr };
        }
        const clock_type t0 = clock_policy_type::get_clock();
        const auto wk_num = base_ult_itf_type::get_worker_num();
        const auto p = MLOG_BEGIN(&this->md_, wk_num, t0);
        return { p };
    }
    template <kind_type Kind>
    void end(const record_type r)
    {
        if (!this->is_enabled_) {
            return;
        }
        const clock_type t1 = clock_policy_type::get_clock();
        const auto wk_num = base_ult_itf_type::get_worker_num();
        MLOG_END(&this->md_, wk_num, r.begin_ptr, &decode_interval, t1, Kind);
    }
    
private:
    static void* decode_interval(FILE* stream, int rank0, int rank1, void* buf0, void* buf1)
    {
        const auto t0 = MLOG_READ_ARG(&buf0, clock_type);
        const auto t1 = MLOG_READ_ARG(&buf1, clock_type);
        const auto kind = MLOG_READ_ARG(&buf1, kind_type);
        
        const auto kind_str = P::get_kind_name(kind);
        
        fprintf(stream, "%d,%" PRId64 ",%d,%" PRId64 ",%s\n",
            rank0, t0, rank1, t1, kind_str);
        //fprintf(stream, "- { kind: %s, rank0: %d, rank1: %d, t0: %" PRId64 ", t1: %" PRId64 " }\n",
        //    kind_str, rank0, rank1, t0, t1);

        return buf1;
    }

public:
    template <kind_type Kind>
    void add() {
        const auto r = this->begin<Kind>();
        this->end<Kind>(r);
    }
    
    void print_all(const char* const module_name, const fdn::size_t proc_id)
    {
        std::stringstream filename_st;
        filename_st << get_prof_output_prefix() << "." << module_name << "." << proc_id;
        const auto filename = filename_st.str();
        
        const auto file = fopen(filename.c_str(), "w");
        mlog_flush_all(&this->md_, file);
        fclose(file);
    }

private:
    static std::string get_prof_output_prefix() noexcept {
        const char* const trace_prefix_str = std::getenv("CMPTH_TRACE_PREFIX");
        return trace_prefix_str != nullptr ? trace_prefix_str : "trace";
    }
    
    bool is_enabled_ = true;
    mlog_data_t md_;
};

} // namespace cmpth

