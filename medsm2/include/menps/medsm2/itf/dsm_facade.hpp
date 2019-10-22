
#pragma once

#include <menps/medsm2/itf/basic_dsm_facade.hpp>
#include <menps/medsm2/com/dsm_com_itf.hpp>
#include <menps/medsm2/svm/svm_space_base.hpp>

namespace menps {
namespace medsm2 {

struct dsm_facade_policy {
    using com_itf_type = dsm_com_itf_t;
    using svm_space_type = svm_space_base;
    using thread_num_type = int;
    
    static thread_num_type get_num_threads_per_proc() {
        static const auto ret = get_num_threads_per_proc_nocache();
        return ret;
    }
    static thread_num_type get_num_threads_per_proc_nocache() {
        if (const char* const str = std::getenv("MEDSM2_NUM_THREADS")) {
            return std::atoi(str);
        }
        else
            return 1;
    }
};

using dsm_facade = basic_dsm_facade<dsm_facade_policy>;

} // namespace medsm2
} // namespace menps

