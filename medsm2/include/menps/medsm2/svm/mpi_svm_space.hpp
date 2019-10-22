
#pragma once

#include <menps/medsm2/svm/svm_space_base.hpp>
#include <menps/medsm2/com/dsm_com_itf.hpp>

namespace menps {
namespace medsm2 {

fdn::unique_ptr<svm_space_base> make_mpi_svm_space(dsm_com_itf_t& com);

} // namespace medsm2
} // namespace menps

