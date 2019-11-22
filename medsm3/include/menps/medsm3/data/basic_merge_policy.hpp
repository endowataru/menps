
#pragma once

#include <menps/medsm3/common.hpp>
//#ifdef MEDSM2_USE_SIMD_DIFF // TODO
    #include <immintrin.h>
//#endif

namespace menps {
namespace medsm3 {

template <typename P>
struct basic_merge_policy
{
private:
    using blk_local_lock_type = typename P::blk_local_lock_type;

    using byte = fdn::byte;
    using size_t = fdn::size_t;

    using constants_type = typename P::constants_type;

public:
    static void read_merge(
        blk_local_lock_type&    blk_llk
    ,   const byte* const       other_pub
    ,         byte* const       my_priv
    ,         byte* const       my_pub
    ,   const size_t            blk_size
    ) {
        CMPTH_P_PROF_SCOPE(P, read_merge);
        if (constants_type::is_simd_diff_enabled) {
            read_merge_simd(blk_llk, other_pub, my_priv, my_pub, blk_size);
        }
        else {
            read_merge_byte(blk_llk, other_pub, my_priv, my_pub, blk_size);
        }
    }

    static void write_merge(
        blk_local_lock_type&    blk_llk
    ,   const byte* const       other_pub
    ,         byte* const       my_priv
    ,         byte* const       my_pub
    ,   const size_t            blk_size
    ,   const bool              use_simd MEFDN_MAYBE_UNUSED
    ) {
        if (constants_type::is_simd_diff_enabled && use_simd) {
            CMPTH_P_PROF_SCOPE(P, write_merge_simd);
            write_merge_simd(blk_llk, other_pub, my_priv, my_pub, blk_size);
        }
        else {
            CMPTH_P_PROF_SCOPE(P, write_merge_byte);
            write_merge_byte(blk_llk, other_pub, my_priv, my_pub, blk_size);
        }
    }
    
private:
    static void read_merge_byte(
        blk_local_lock_type&    blk_llk
    ,   const byte* const       other_pub
    ,         byte* const       my_priv
    ,         byte* const       my_pub
    ,   const size_t            blk_size
    ) {
        for (size_t i = 0; i < blk_size; ++i) {
            // Check that there is no remote modification on this block.
            if (my_pub[i] == other_pub[i]) {
                // Preserve the diff between my_pub and my_priv
                // to merge them in the next transaction.
            }
            else {
                if (constants_type::is_race_detection_enabled) {
                    if (my_priv[i] != other_pub[i]) {
                        report_data_race(blk_llk, i, other_pub, my_priv, my_pub, blk_size);
                    }
                }
                // Other processes have modified this block.
                my_pub[i] = other_pub[i];
                my_priv[i] = other_pub[i];
            }
        }
    }
    
    static void write_merge_byte(
        blk_local_lock_type&    blk_llk
    ,   const byte* const       other_pub
    ,         byte* const       my_priv
    ,         byte* const       my_pub
    ,   const size_t            blk_size
    ) {
        for (size_t i = 0; i < blk_size; ++i) {
            // Check that there is no remote modification on this block.
            if (my_pub[i] == other_pub[i]) {
                // Any changes prior to this merge are
                // considered as merged in this transaction.
                my_pub[i] = my_priv[i];
            }
            else {
                if (constants_type::is_race_detection_enabled) {
                    if (my_priv[i] != my_pub[i]) {
                        report_data_race(blk_llk, i, other_pub, my_priv, my_pub, blk_size);
                    }
                }
                // Other processes have modified this block.
                my_pub[i] = other_pub[i];
                my_priv[i] = other_pub[i];
            }
        }
    }
    
    //#ifdef MEDSM2_USE_SIMD_DIFF
    static void read_merge_simd(
        blk_local_lock_type&    blk_llk
    ,   const byte* const       other_pub
    ,         byte* const       my_priv
    ,         byte* const       my_pub
    ,   const size_t            blk_size
    ) {
        merge_simd(blk_llk, other_pub, my_priv, my_pub, blk_size, false);
    }
    static void write_merge_simd(
        blk_local_lock_type&    blk_llk
    ,   const byte* const       other_pub
    ,         byte* const       my_priv
    ,         byte* const       my_pub
    ,   const size_t            blk_size
    ) {
        merge_simd(blk_llk, other_pub, my_priv, my_pub, blk_size, true);
    }
    
    static void merge_simd(
        blk_local_lock_type&    blk_llk
    ,   const byte* const       other_pub
    ,         byte* const       my_priv
    ,         byte* const       my_pub
    ,   const size_t            blk_size
    ,   const bool              is_write
    ) {
        using vec_buf_type = __m256i;
        constexpr auto vector_len = sizeof(vec_buf_type);
        constexpr auto unroll_len = constants_type::merge_simd_unroll_len; // = 8
        
        MEFDN_ASSERT(blk_size % (vector_len * unroll_len) == 0);
        
        auto* other_pub_vec = reinterpret_cast<const vec_buf_type*>(other_pub);
        auto* my_priv_vec   = reinterpret_cast<vec_buf_type*>(my_priv);
        auto* my_pub_vec    = reinterpret_cast<vec_buf_type*>(my_pub);
        
        // __asm__ __volatile__ ("# merge_simd() starts");
        
        for (size_t i = 0; i < blk_size / vector_len; i += unroll_len) {
            vec_buf_type other_pub_regs[unroll_len];
            vec_buf_type my_priv_regs[unroll_len];
            vec_buf_type my_pub_regs[unroll_len];
            vec_buf_type result_regs[unroll_len];
            
            int dr_flags[unroll_len] MEFDN_MAYBE_UNUSED;
            
            for (size_t j = 0; j < unroll_len; j++) {
                other_pub_regs[j] = _mm256_loadu_si256(&other_pub_vec[i+j]); // TODO
                my_priv_regs[j]   = my_priv_vec[i+j];
                my_pub_regs[j]    = my_pub_vec[i+j];
            }
            
            for (size_t j = 0; j < unroll_len; j++) {
                const auto other_diff = other_pub_regs[j] ^ my_pub_regs[j];
                const auto my_diff    = my_priv_regs[j]   ^ my_pub_regs[j];
                
                result_regs[j] = my_diff ^ other_pub_regs[j];
                
                if (constants_type::is_race_detection_enabled) {
                    // True if (other_diff & my_diff) == 0.
                    dr_flags[j] = _mm256_testz_si256(other_diff, my_diff);
                }
            }
            
            if (constants_type::is_race_detection_enabled) {
                bool dr_flag = dr_flags[0];
                for (size_t j = 1; j < unroll_len; ++j) {
                    dr_flag = dr_flag && (dr_flags[j] != 0);
                }
                if (MEFDN_UNLIKELY(!dr_flag)) {
                    report_data_race(blk_llk, 0, other_pub, my_priv, my_pub, blk_size);
                }
            }
            
            for (size_t j = 0; j < unroll_len; j++) {
                const auto new_pub_vec =
                    is_write ? result_regs[j] : other_pub_regs[j];
                
                #if 1
                _mm256_stream_si256(&my_priv_vec[i+j], result_regs[j]);
                _mm256_stream_si256(&my_pub_vec[i+j] , new_pub_vec);
                #else
                my_priv_vec[i+j] = result_regs[j];
                my_pub_vec[i+j]  = new_pub_vec;
                #endif
            }
        }
        
        // __asm__ __volatile__ ("# merge_simd() ends");
    }
    //#endif
    
    static void report_data_race(
        blk_local_lock_type&    blk_llk
    ,   const size_t            index
    ,   const byte* const       other_pub
    ,         byte* const       my_priv
    ,         byte* const       my_pub
    ,   const size_t            blk_size
    ) {
        for (size_t i = 0; i < blk_size; ++i) {
            if ((other_pub[i] != my_pub[i]) && (my_priv[i] != my_pub[i])) {
                fmt::memory_buffer w;
                format_to(w, "msg:Detected data race.\t");
                format_to(w, "seg_id:{}\t", blk_llk.seg_id());
                format_to(w, "blk_sidx:{}\t", blk_llk.subindex());
                format_to(w, "index:{}\t", index);
                format_to(w, "other_pub:0x{:x}\t", reinterpret_cast<fdn::intptr_t>(&other_pub[i]));
                format_to(w, "my_priv:0x{:x}\t"  , reinterpret_cast<fdn::intptr_t>(&my_priv[i]));
                format_to(w, "my_pub:0x{:x}\t"   , reinterpret_cast<fdn::intptr_t>(&my_pub[i]));
                
                format_to(w, "other_pub_1b:0x{:x}\t", get_aligned_data_race_val<fdn::uint8_t>(&other_pub[i]));
                format_to(w, "my_priv_1b:0x{:x}\t"  , get_aligned_data_race_val<fdn::uint8_t>(&my_priv[i]));
                format_to(w, "my_pub_1b:0x{:x}\t"   , get_aligned_data_race_val<fdn::uint8_t>(&my_pub[i]));
                
                format_to(w, "other_pub_2b:0x{:x}\t", get_aligned_data_race_val<fdn::uint16_t>(&other_pub[i]));
                format_to(w, "my_priv_2b:0x{:x}\t"  , get_aligned_data_race_val<fdn::uint16_t>(&my_priv[i]));
                format_to(w, "my_pub_2b:0x{:x}\t"   , get_aligned_data_race_val<fdn::uint16_t>(&my_pub[i]));
                
                format_to(w, "other_pub_4b:0x{:x}\t", get_aligned_data_race_val<fdn::uint32_t>(&other_pub[i]));
                format_to(w, "my_priv_4b:0x{:x}\t"  , get_aligned_data_race_val<fdn::uint32_t>(&my_priv[i]));
                format_to(w, "my_pub_4b:0x{:x}\t"   , get_aligned_data_race_val<fdn::uint32_t>(&my_pub[i]));
                
                format_to(w, "other_pub_8b:0x{:x}\t", get_aligned_data_race_val<fdn::uint64_t>(&other_pub[i]));
                format_to(w, "my_priv_8b:0x{:x}\t"  , get_aligned_data_race_val<fdn::uint64_t>(&my_priv[i]));
                format_to(w, "my_pub_8b:0x{:x}\t"   , get_aligned_data_race_val<fdn::uint64_t>(&my_pub[i]));
                
                const auto s = to_string(w);
                CMPTH_P_LOG_FATAL(P, "Detected data race.", "detail", s);
                throw std::runtime_error(s);
            }
        }
        
        // Lost the exact position...
        fmt::memory_buffer w;
        format_to(w, "msg:Detected data race? (but lost exact position...)\t");
        format_to(w, "seg_id:{}\t", blk_llk.seg_id());
        format_to(w, "blk_sidx:{}\t", blk_llk.subindex());
        format_to(w, "index:{}\t", index);
        format_to(w, "other_pub:0x{:x}\t", reinterpret_cast<mefdn::intptr_t>(other_pub));
        format_to(w, "my_priv:0x{:x}\t"  , reinterpret_cast<mefdn::intptr_t>(my_priv));
        format_to(w, "my_pub:0x{:x}"     , reinterpret_cast<mefdn::intptr_t>(my_pub));
        
        const auto s = to_string(w);
        CMPTH_P_LOG_FATAL(P, "Detected data race.", "detail", s);
        throw std::logic_error(s);
    }
    
    template <typename T>
    static T get_aligned_data_race_val(const fdn::byte* const p) {
        auto ret_pi = reinterpret_cast<fdn::uintptr_t>(p);
        
        // Align the pointer.
        ret_pi -= ret_pi % sizeof(T);
        
        return *reinterpret_cast<T*>(ret_pi);
    }
};

} // namespace medsm3
} // namespace menps

