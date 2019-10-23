
#include <menps/medsm3/md3/def_md3_itf.hpp>
#include <menps/medsm3/md3/make_md3_space.hpp>

namespace menps {
namespace medsm3 {

fdn::unique_ptr<medsm2::svm_space_base> make_md3_space(medsm2::dsm_com_itf_t& com)
{
    using md3_itf = def_lv5_md3_itf;

    // lv1
    const fdn::size_t max_num_segs = (MEDSM2_MAX_SPACE_SIZE)/(MEDSM2_MAX_SEG_SIZE); // TODO
    auto seg_set_ptr = std::make_shared<md3_itf::segment_set_type>(com, max_num_segs);
    
    // lv2
    auto wr_count_ctrl_ptr = fdn::make_unique<md3_itf::wr_count_ctrl_type>(seg_set_ptr);
    auto sd_ctrl_ptr =
        fdn::make_unique<md3_itf::state_data_ctrl_type>(
            fdn::make_unique<md3_itf::state_ctrl_type>(
                seg_set_ptr
            ,   fdn::move(wr_count_ctrl_ptr)
            )
        ,   fdn::make_unique<md3_itf::data_ctrl_type>(seg_set_ptr)
        );

    // lv3
    auto rd_ctrl =
        fdn::make_unique<md3_itf::rd_ctrl_type>(
            fdn::make_unique<md3_itf::local_lock_ctrl_type>(seg_set_ptr)
        ,   fdn::make_unique<md3_itf::home_ctrl_type>(seg_set_ptr)
        ,   fdn::move(sd_ctrl_ptr)
        ,   fdn::make_unique<md3_itf::inv_ctrl_type>(seg_set_ptr)
        );

    // lv4
    auto wr_ctrl = fdn::make_unique<md3_itf::wr_ctrl_type>(seg_set_ptr, fdn::move(rd_ctrl));
    auto pin_ctrl = fdn::make_unique<md3_itf::pin_ctrl_type>(fdn::move(wr_ctrl));

    // lv5
    auto sync_tbl =
        fdn::make_unique<md3_itf::sync_table_type>(
            com
        ,   1024 // TODO: magic number 
        );
    
    return fdn::make_unique<def_lv5_md3_itf::space>(
        fdn::move(seg_set_ptr), fdn::move(pin_ctrl), fdn::move(sync_tbl)
    );
}

} // namespace medsm3
} // namespace menps

