
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class dir_rel_sig
{
    using wn_vector_type = typename P::wn_vector_type;
    using sig_buffer_type = typename P::sig_buffer_type;
    
public:
    template <typename SegTable>
    void merge(SegTable& /*seg_tbl*/, wn_vector_type /*wn_vec*/) {
        // Do nothing.
    }
    
    sig_buffer_type get_sig(const bool /*clear_sig*/) const {
        return sig_buffer_type();
    }
};

} // namespace medsm3
} // namespace menps

