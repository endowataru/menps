
#pragma once

#include <menps/medsm3/md3/lv1_md3_itf.hpp>
#include <menps/medsm3/md3/lv2_md3_itf.hpp>
#include <menps/medsm3/md3/lv3_md3_itf.hpp>
#include <menps/medsm3/md3/lv4_md3_itf.hpp>
#include <menps/medsm3/md3/lv5_md3_itf.hpp>
#include <menps/medsm2/com/dsm_com_itf.hpp>
#include <menps/medsm2/svm/svm_space_base.hpp>

namespace menps {
namespace medsm3 {

// level 1

struct def_lv1_md3_itf_policy
{
    using com_itf_type = medsm2::dsm_com_itf_t;
    using mtx_id_type = medsm2::svm_space_base::mutex_id_t;
    using prof_aspect_type = medsm2::dsm_com_policy_base::prof_aspect_type;

    using log_aspect_type =
        typename com_itf_type::ult_itf_type::log_aspect::template rebind<32>;

    struct constants_type {
        static const fdn::uint64_t lease_ts = 10;

        static const bool use_directory_coherence =
            #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
            true
            #else
            false
            #endif
            ;
        static const fdn::size_t max_rel_sig_len =
            use_directory_coherence ? 0 : MEDSM2_REL_SIG_LEN;

        static const bool is_migration_enabled =
            #ifdef MEDSM2_ENABLE_MIGRATION
            true
            #else
            false
            #endif
            ;
        static const bool is_lazy_merge_enabled =
            #ifdef MEDSM2_ENABLE_LAZY_MERGE
            true
            #else
            false
            #endif
            ;
        static const bool is_fast_release_enabled =
            #ifdef MEDSM2_ENABLE_FAST_RELEASE
            true
            #else
            false
            #endif
            ;
        static const bool is_needs_local_comp_enabled =
            #ifdef MEDSM2_ENABLE_NEEDS_LOCAL_COMP
            true
            #else
            false
            #endif
            ;
        static const bool is_signature_enabled = true;

        static const fdn::size_t wr_count_threshold = MEDSM2_WR_COUNT_THRESHOLD;
        static const fdn::size_t max_fast_rel_threshold = MEDSM2_MAX_FAST_REL_THRESHOLD;

        static const fdn::size_t ln2_max_seg_size = 38; // TODO

        static const bool is_simd_diff_enabled =
            #ifdef MEDSM2_USE_SIMD_DIFF
            true
            #else
            false
            #endif
            ;
        static const bool is_race_detection_enabled =
            #ifdef MEDSM2_ENABLE_RACE_DETECTION
            true
            #else
            false
            #endif
            ;
        static const fdn::size_t merge_simd_unroll_len = 8; // TODO

        static const bool enable_fault_classification =
            #ifdef MEDSM2_ENABLE_FAULT_CLASSIFICATION
            true
            #else
            false
            #endif
            ;
    };
};

using def_lv1_md3_itf = lv1_md3_itf<def_lv1_md3_itf_policy>;

// level 2

struct def_lv2_md3_itf_policy
{
    using lv1_itf_type = def_lv1_md3_itf;
};

using def_lv2_md3_itf = lv2_md3_itf<def_lv2_md3_itf_policy>;

// level 3

struct def_lv3_md3_itf_policy
{
    using lv2_itf_type = def_lv2_md3_itf;
};

using def_lv3_md3_itf = lv3_md3_itf<def_lv3_md3_itf_policy>;

// level 4

struct def_lv4_md3_itf_policy
{
    using lv3_itf_type = def_lv3_md3_itf;
};

using def_lv4_md3_itf = lv4_md3_itf<def_lv4_md3_itf_policy>;

// level 5

struct def_lv5_md3_itf_policy
{
    using lv4_itf_type = def_lv4_md3_itf;
    using space_base_type = medsm2::svm_space_base;
};

using def_lv5_md3_itf = lv5_md3_itf<def_lv5_md3_itf_policy>;

} // namespace medsm3
} // namespace menps

