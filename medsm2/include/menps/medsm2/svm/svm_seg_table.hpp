
#pragma once

#include <menps/medsm2/seg_table.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class svm_seg_table;

template <typename P>
struct svm_seg_table_policy : P
{
    using derived_type = svm_seg_table<P>;
};

template <typename P>
class svm_seg_table
    : public seg_table<svm_seg_table_policy<P>>
{
    using base = seg_table<svm_seg_table_policy<P>>;
    
    using blk_tbl_type = typename P::blk_tbl_type;
    
    using seg_id_type = typename P::seg_id_type;
    using blk_id_type = typename P::blk_id_type;
    using blk_pos_type = typename P::blk_pos_type;
    
    using size_type = typename P::size_type;
    
public:
    template <typename Conf>
    explicit svm_seg_table(const Conf& conf)
        : base(conf)
        //, max_seg_size_(conf.max_seg_size)
        , max_seg_size_(P::constants_type::max_seg_size) // TODO
        , ln2_max_seg_size_(mefdn::floor_log2(max_seg_size_))
    {
        MEFDN_ASSERT(mefdn::is_power_of_2(max_seg_size_));
    }
    
private:
    friend seg_table<svm_seg_table_policy<P>>;
    
    seg_id_type get_seg_id(const blk_id_type blk_id)
    {
        // Shift the block ID to divide.
        const auto ret = blk_id >> this->ln2_max_seg_size_;
        
        // Check that the division is correct.
        MEFDN_ASSERT(ret == blk_id / max_seg_size_);
        
        return ret;
    }
    
    const size_type max_seg_size_;
    const size_type ln2_max_seg_size_;
};

} // namespace medsm2
} // namespace menps

