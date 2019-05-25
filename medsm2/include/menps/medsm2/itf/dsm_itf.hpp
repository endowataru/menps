
#pragma once

#include <menps/medsm2/itf/basic_dsm_itf.hpp>
#include <menps/medsm2/itf/dsm_facade.hpp>

namespace menps {
namespace medsm2 {

struct dsm_itf_policy {
    using dsm_facade_type = dsm_facade;
    using ult_itf_type = typename dsm_com_creator::ult_itf_type;
    using thread_num_type = int;
};

using dsm_itf = basic_dsm_itf<dsm_itf_policy>;

} // namespace medsm2
} // namespace menps

