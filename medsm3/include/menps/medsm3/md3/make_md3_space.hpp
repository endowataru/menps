
#pragma once

#include <menps/medsm3/common.hpp>
#include <menps/medsm2/com/dsm_com_itf.hpp>
#include <menps/medsm2/svm/svm_space_base.hpp>

namespace menps {
namespace medsm3 {

fdn::unique_ptr<medsm2::svm_space_base> make_md3_space(medsm2::dsm_com_itf_t& com);

} // namespace medsm3
} // namespace menps

