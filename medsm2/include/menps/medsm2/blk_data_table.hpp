
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/logger.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <cstring>

#ifdef MEDSM2_USE_SIMD_DIFF
    #include <immintrin.h>
#endif

namespace menps {
namespace medsm2 {

#define MEDSM2_USE_COMPARE_DIFF
//#define MEDSM2_DISABLE_READ_MERGE
//#define MEDSM2_FORCE_ALWAYS_MERGE_LOCAL
//#define MEDSM2_FORCE_ALWAYS_MERGE_REMOTE

class data_race_error
    : public std::runtime_error
{
    using base = std::runtime_error;
    
public:
    using base::base;
};

template <typename P>
class blk_data_table
{
    MEFDN_DEFINE_DERIVED(P)
    
    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    using blk_pos_type = typename P::blk_pos_type;
    
    using unique_lock_type = typename P::unique_lock_type;
    
    using size_type = typename P::size_type;
    
public:
    template <typename Conf>
    void coll_make(const Conf& conf)
    {
        const auto priv = static_cast<mefdn::byte*>(conf.priv_buf);
        
        // Attach the private buffer.
        this->priv_ptr_ = conf.com.get_rma().attach(priv, priv + conf.seg_size);
        
        #ifdef MEDSM2_ENABLE_LAZY_MERGE
        this->priv_buf_.coll_make(conf.com.get_rma(), conf.com.get_coll(), this->priv_ptr_, conf.seg_size);
        #endif
        
        const auto pub = static_cast<mefdn::byte*>(conf.pub_buf);
        
        // Attach the public buffer.
        this->pub_ptr_ = conf.com.get_rma().attach(pub, pub + conf.seg_size);
        
        this->pub_buf_.coll_make(conf.com.get_rma(), conf.com.get_coll(), this->pub_ptr_, conf.seg_size);
    }
    
    void finalize(com_itf_type& com)
    {
        // Detach the public buffer.
        com.get_rma().detach(this->pub_ptr_);
        
        // Detach the public buffer.
        com.get_rma().detach(this->priv_ptr_);
    }
    
    void* get_pub_ptr() const noexcept {
        return this->get_my_pub_ptr(0);
    }
    
    void start_read(
        com_itf_type&           com
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const proc_id_type      home_proc
    ,   const bool              is_dirty
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto blk_size = self.get_blk_size();
        const auto cur_proc = com.this_proc_id();
        auto& rma = com.get_rma();
        
        const auto my_priv = this->get_my_priv_ptr(blk_pos);
        
        if (cur_proc != home_proc) {
            #ifndef MEDSM2_DISABLE_READ_MERGE
            if (is_dirty) {
                const auto p = prof::start();
                
                // Merge the diff in the read.
                
                // Read the data of the home process into a temporary buffer.
                const auto home_data_buf =
                    rma.buf_read(
                        home_proc
                    #ifdef MEDSM2_ENABLE_LAZY_MERGE
                    ,   this->get_other_priv_ptr(home_proc, blk_pos)
                    #else
                    ,   this->get_other_pub_ptr(home_proc, blk_pos)
                    #endif
                    ,   blk_size
                    );
                
                
                const auto home_data = home_data_buf.get();
                
                const auto my_pub = this->get_my_pub_ptr(blk_pos);
                
                // Apply the changes written in the home into my_priv and my_pub.
                this->read_merge(blk_pos, home_data, my_priv, my_pub, blk_size);
                
                prof::finish(prof_kind::merge_read, p);
            }
            else {
            #endif
                const auto p = prof::start();
                
                // Simply read from the home process.
                rma.read(
                    home_proc
                #ifdef MEDSM2_ENABLE_LAZY_MERGE
                ,   this->get_other_priv_ptr(home_proc, blk_pos)
                #else
                ,   this->get_other_pub_ptr(home_proc, blk_pos)
                #endif
                ,   my_priv
                ,   blk_size
                );
                
                prof::finish(prof_kind::wn_read, p);
                
            #ifndef MEDSM2_DISABLE_READ_MERGE
            }
            #endif
        }
        
        const auto p = prof::start();
        
        // Call mprotect(PROT_READ).
        self.set_readonly(blk_pos, blk_size);
        
        prof::finish(prof_kind::mprotect_start_read, p);
    }
    
    void start_write(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const bool              needs_twin
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto blk_size = self.get_blk_size();
        
        const auto my_priv = this->get_my_priv_ptr(blk_pos);
        const auto my_pub = this->get_my_pub_ptr(blk_pos);
        
        if (needs_twin) {
            const auto p = prof::start();
            
            // Copy the private data to the public data.
            // This is a preparation for releasing this block later.
            std::memcpy(my_pub, my_priv, blk_size);
            //std::copy(my_priv, my_priv + blk_size, my_pub);
            
            prof::finish(prof_kind::start_write_twin, p);
        }
        
        const auto p = prof::start();
        
        // Call mprotect(PROT_READ | PROT_WRITE).
        self.set_writable(blk_pos, blk_size);
        
        prof::finish(prof_kind::mprotect_start_write, p);
    }
    
    struct release_merge_result {
        // This block was written by this process
        // and must be recorded to the write notices.
        bool is_written;
    };
    
    template <typename LockResult>
    release_merge_result release_merge(
        com_itf_type&           com
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const LockResult&       glk_ret
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto blk_size = self.get_blk_size();
        auto& rma = com.get_rma();
        
        if (glk_ret.needs_protect_before) {
            // Only when the block was writable and should be protected,
            // this method write-protects this block
            // in order to apply the changes to the private data.
            
            const auto p = prof::start();
            
            // Call mprotect(PROT_READ).
            self.set_readonly(blk_pos, blk_size);
            
            prof::finish(prof_kind::mprotect_tx_before, p);
        }
        
        const auto my_priv = this->get_my_priv_ptr(blk_pos);
        const auto my_pub = this->get_my_pub_ptr(blk_pos);
        
        bool is_written = false;
        
        if (glk_ret.is_dirty) {
            // Compare the private data with the public data.
            // Note that the private data is STILL WRITABLE
            // and can be modified concurrently by other threads in this process.
            // It's OK to read the intermediate states
            // because those writes will be managed by the next release operation.
            if (glk_ret.needs_local_comp){
                const auto p = prof::start();
                
                is_written = std::memcmp(my_priv, my_pub, blk_size) != 0;
                //std::equal(my_priv, my_priv + blk_size, my_pub)
                
                prof::finish(prof_kind::tx_merge_memcmp, p);
            }
            else {
                is_written = true;
            }
        }
        
        if (! glk_ret.is_remotely_updated) {
            #ifndef MEDSM2_FORCE_ALWAYS_MERGE_LOCAL
            if (is_written) {
            #endif
                if (glk_ret.needs_local_copy) {
                    const auto p = prof::start();
                    
                    // Copy the private data to the public data.
                    std::memcpy(my_pub, my_priv, blk_size);
                    //std::copy(my_priv, my_priv + blk_size, my_pub);
                    
                    prof::finish(prof_kind::tx_merge_local_memcpy, p);
                }
            #ifndef MEDSM2_FORCE_ALWAYS_MERGE_LOCAL
            }
            else {
                // The data is not modified.
            }
            #endif
        }
        else {
            const auto cur_owner = glk_ret.owner;
            
            // Read the data from cur_owner.
            const auto other_data_buf =
                rma.buf_read(
                    cur_owner
                #ifdef MEDSM2_ENABLE_LAZY_MERGE
                ,   this->get_other_priv_ptr(cur_owner, blk_pos)
                #else
                ,   this->get_other_pub_ptr(cur_owner, blk_pos)
                #endif
                ,   blk_size
                );
            
            #ifdef MEDSM2_ENABLE_LAZY_MERGE
            // Write back to cur_owner.
            rma.write(
                cur_owner
            ,   this->get_other_pub_ptr(cur_owner, blk_pos)
            ,   other_data_buf.get()
            ,   blk_size
            );
            #endif
            
            const auto other_data = other_data_buf.get();
            
            #ifndef MEDSM2_FORCE_ALWAYS_MERGE_REMOTE
            if (is_written) {
            #endif
                // Use SIMD if the private data is write-protected.
                const bool use_simd = glk_ret.is_write_protected;
                
                // Three copies (my_pub, my_priv, other_data) are different with each other.
                // It is necessary to merge them to complete the release.
                this->write_merge(blk_pos, other_data, my_priv, my_pub, blk_size, use_simd);
                
            #ifndef MEDSM2_FORCE_ALWAYS_MERGE_REMOTE
            }
            else {
                const auto p = prof::start();
                
                // Although this process doesn't release this block at this time,
                // the buffer read from the current owner can be utilized.
                // This is important when an acquire on this block is on-going
                // because that thread requires this releaser thread
                // to make the latest modifications visible on this process.
                // Note: The timestamp should also be updated in the directory later.
                std::memcpy(my_priv, other_data, blk_size);
                //std::copy(other_data, other_data + blk_size, my_priv);
                std::memcpy(my_pub , other_data, blk_size);
                //std::copy(other_data, other_data + blk_size, my_pub);
                
                prof::finish(prof_kind::tx_merge_remote_memcpy, p);
            }
            #endif
        }
        
        if (glk_ret.needs_protect_after) {
            const auto p = prof::start();
            
            // If this block was inaccessible (invalid-clean or invalid-dirty) from the application,
            // make the block readable now.
            
            // Call mprotect(PROT_READ).
            self.set_readonly(blk_pos, blk_size);
            
            prof::finish(prof_kind::mprotect_tx_after, p);
        }
        
        return { is_written };
        
        // The temporary buffer is discarded in its destructor here.
    }
    
    void invalidate(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        const auto blk_size = self.get_blk_size();
        
        const auto p = prof::start();
        
        // Call mprotect(PROT_NONE).
        self.set_inaccessible(blk_pos, blk_size);
        
        prof::finish(prof_kind::mprotect_invalidate, p);
    }
    
    // This function is used to implement atomic operations.
    template <typename Func>
    void perform_transaction(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   Func&&                  func
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto my_priv = this->get_my_priv_ptr(blk_pos);
        const auto my_pub = this->get_my_pub_ptr(blk_pos);
        
        // Perform a transaction on this block.
        mefdn::forward<Func>(func)(my_priv, my_pub);
    }
    
    template <typename T, typename Func>
    bool do_amo_at(
        const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const size_type         offset
    ,   Func&&                  func
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto my_priv = this->get_my_priv_ptr(blk_pos);
        const auto my_pub = this->get_my_pub_ptr(blk_pos);
        
        auto target_priv =
            reinterpret_cast<T*>(
                static_cast<mefdn::byte*>(my_priv) + offset
            );
        auto target_pub =
            reinterpret_cast<T*>(
                static_cast<mefdn::byte*>(my_pub) + offset
            );
        
        // Load the atomic variable.
        const auto target = *target_pub;
        // This block must be already clean.
        MEFDN_ASSERT(*target_priv == target);
        
        // Execute the atomic operation.
        const auto result =
            mefdn::forward<Func>(func)(target);
        
        // Check whether this atomic variable is modified.
        const bool is_changed = (target != result);
        
        if (is_changed) {
            // Assign the modified value to both the private and public data.
            *target_priv = result;
            *target_pub  = result;
        }
        
        return is_changed;
    }
    
    
private:
    static void read_merge(
        const blk_pos_type          blk_pos
    ,   const mefdn::byte* const    other_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ) {
        const auto p = prof::start();
        
        #ifdef MEDSM2_USE_SIMD_DIFF
        read_merge_simd(blk_pos, other_pub, my_priv, my_pub, blk_size);
        #else
        read_merge_byte(blk_pos, other_pub, my_priv, my_pub, blk_size);
        #endif
        
        prof::finish(prof_kind::read_merge, p);
    }
    
    static void write_merge(
        const blk_pos_type          blk_pos
    ,   const mefdn::byte* const    other_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ,   const bool                  use_simd MEFDN_MAYBE_UNUSED
    ) {
        #ifdef MEDSM2_USE_SIMD_DIFF
        if (use_simd) {
            const auto p = prof::start();
            
            write_merge_simd(blk_pos, other_pub, my_priv, my_pub, blk_size);
            
            prof::finish(prof_kind::write_merge_simd, p);
        }
        else {
        #endif
            const auto p = prof::start();
            
            write_merge_byte(blk_pos, other_pub, my_priv, my_pub, blk_size);
            
            prof::finish(prof_kind::write_merge_byte, p);
        #ifdef MEDSM2_USE_SIMD_DIFF
        }
        #endif
    }
    
    static void read_merge_byte(
        const blk_pos_type          blk_pos
    ,   const mefdn::byte* const    home_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ) {
        for (size_type i = 0; i < blk_size; ++i) {
            #ifdef MEDSM2_USE_COMPARE_DIFF
            if (my_priv[i] != my_pub[i]) {
                #ifdef MEDSM2_ENABLE_RACE_DETECTION
                if (my_pub[i] != home_pub[i]) {
                    report_data_race(blk_pos, i, home_pub, my_priv, my_pub, blk_size);
                }
                #endif
            }
            else {
                my_pub[i] = home_pub[i];
                my_priv[i] = home_pub[i];
            }
            
            #else
            // TODO: Scoped enums cannot do XOR...
            const auto changed =
                static_cast<unsigned char>(my_pub[i]) ^
                static_cast<unsigned char>(home_pub[i])
            
            my_priv[i] =
                static_cast<mefdn::byte>(
                    static_cast<unsigned char>(my_priv[i]) ^ changed
                );
            my_pub[i] = home_pub[i];
            #endif
        }
    }
    
    static void write_merge_byte(
        const blk_pos_type          blk_pos
    ,   const mefdn::byte* const    other_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ) {
        for (size_type i = 0; i < blk_size; ++i) {
            #ifdef MEDSM2_USE_COMPARE_DIFF
            if (my_priv[i] != my_pub[i]) {
                #ifdef MEDSM2_ENABLE_RACE_DETECTION
                if (my_pub[i] != other_pub[i]) {
                    report_data_race(blk_pos, i, other_pub, my_priv, my_pub, blk_size);
                }
                #endif
                my_pub[i] = my_priv[i];
            }
            else {
                my_pub[i] = other_pub[i];
                my_priv[i] = other_pub[i];
            }
            
            #else
            // TODO: Scoped enums cannot do XOR...
            my_priv[i] =
                static_cast<mefdn::byte>(
                    static_cast<unsigned char>(my_priv[i]) ^
                    static_cast<unsigned char>(my_pub[i]) ^
                    static_cast<unsigned char>(other_pub[i])
                );
            my_pub[i] = my_priv[i];
            #endif
        }
    }
    
    #ifdef MEDSM2_USE_SIMD_DIFF
    static void read_merge_simd(
        const blk_pos_type          blk_pos
    ,   const mefdn::byte* const    other_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ) {
        merge_simd(blk_pos, other_pub, my_priv, my_pub, blk_size, false);
    }
    
    static void write_merge_simd(
        const blk_pos_type          blk_pos
    ,   const mefdn::byte* const    other_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ) {
        merge_simd(blk_pos, other_pub, my_priv, my_pub, blk_size, true);
    }
    
    static void merge_simd(
        const blk_pos_type          blk_pos
    ,   const mefdn::byte* const    other_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ,   const bool                  is_write
    ) {
        using vec_buf_type = __m256i;
        #define VECTOR_LEN  sizeof(vec_buf_type)
        #define UNROLL_LEN  8
        
        MEFDN_ASSERT(blk_size % (VECTOR_LEN * UNROLL_LEN) == 0);
        
        auto* other_pub_vec = reinterpret_cast<const vec_buf_type*>(other_pub);
        auto* my_priv_vec   = reinterpret_cast<vec_buf_type*>(my_priv);
        auto* my_pub_vec    = reinterpret_cast<vec_buf_type*>(my_pub);
        
        // __asm__ __volatile__ ("# merge_simd() starts");
        
        for (size_type i = 0; i < blk_size / VECTOR_LEN; i += UNROLL_LEN) {
            vec_buf_type other_pub_regs[UNROLL_LEN];
            vec_buf_type my_priv_regs[UNROLL_LEN];
            vec_buf_type my_pub_regs[UNROLL_LEN];
            vec_buf_type result_regs[UNROLL_LEN];
            
            #ifdef MEDSM2_ENABLE_RACE_DETECTION
            int dr_flags[UNROLL_LEN];
            #endif
            
            for (size_type j = 0; j < UNROLL_LEN; j++) {
                other_pub_regs[j] = _mm256_loadu_si256(&other_pub_vec[i+j]); // TODO
                my_priv_regs[j]   = my_priv_vec[i+j];
                my_pub_regs[j]    = my_pub_vec[i+j];
            }
            
            for (size_type j = 0; j < UNROLL_LEN; j++) {
                const auto other_diff = other_pub_regs[j] ^ my_pub_regs[j];
                const auto my_diff    = my_priv_regs[j]   ^ my_pub_regs[j];
                
                result_regs[j] = my_diff ^ other_pub_regs[j];
                
                #ifdef MEDSM2_ENABLE_RACE_DETECTION
                // True if (other_diff & my_diff) == 0.
                dr_flags[j] = _mm256_testz_si256(other_diff, my_diff);
                #endif
            }
            
            #ifdef MEDSM2_ENABLE_RACE_DETECTION
            bool dr_flag = dr_flags[0];
            for (size_type j = 1; j < UNROLL_LEN; ++j) {
                dr_flag = dr_flag && (dr_flags[j] != 0);
            }
            if (MEFDN_UNLIKELY(!dr_flag)) {
                report_data_race(blk_pos, 0, other_pub, my_priv, my_pub, blk_size);
            }
            #endif
            
            for (size_type j = 0; j < UNROLL_LEN; j++) {
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
        
        #undef VECTOR_LEN
        #undef UNROLL_LEN
    }
    #endif
    
    static void report_data_race(
        const blk_pos_type          blk_pos
    ,   const size_type             index
    ,   const mefdn::byte* const    other_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ) {
        for (size_type i = 0; i < blk_size; ++i) {
            if ((other_pub[i] != my_pub[i]) && (my_priv[i] != my_pub[i])) {
                fmt::MemoryWriter w;
                w.write("msg:Detected data race.\t");
                w.write("blk_pos:{}\t", blk_pos);
                w.write("index:{}\t", index);
                w.write("other_pub:0x{:x}\t", reinterpret_cast<mefdn::intptr_t>(&other_pub[i]));
                w.write("my_priv:0x{:x}\t"  , reinterpret_cast<mefdn::intptr_t>(&my_priv[i]));
                w.write("my_pub:0x{:x}\t"   , reinterpret_cast<mefdn::intptr_t>(&my_pub[i]));
                
                w.write("other_pub_1b:0x{:x}\t", get_aligned_data_race_val<mefdn::uint8_t>(&other_pub[i]));
                w.write("my_priv_1b:0x{:x}\t"  , get_aligned_data_race_val<mefdn::uint8_t>(&my_priv[i]));
                w.write("my_pub_1b:0x{:x}\t"   , get_aligned_data_race_val<mefdn::uint8_t>(&my_pub[i]));
                
                w.write("other_pub_2b:0x{:x}\t", get_aligned_data_race_val<mefdn::uint16_t>(&other_pub[i]));
                w.write("my_priv_2b:0x{:x}\t"  , get_aligned_data_race_val<mefdn::uint16_t>(&my_priv[i]));
                w.write("my_pub_2b:0x{:x}\t"   , get_aligned_data_race_val<mefdn::uint16_t>(&my_pub[i]));
                
                w.write("other_pub_4b:0x{:x}\t", get_aligned_data_race_val<mefdn::uint32_t>(&other_pub[i]));
                w.write("my_priv_4b:0x{:x}\t"  , get_aligned_data_race_val<mefdn::uint32_t>(&my_priv[i]));
                w.write("my_pub_4b:0x{:x}\t"   , get_aligned_data_race_val<mefdn::uint32_t>(&my_pub[i]));
                
                w.write("other_pub_8b:0x{:x}\t", get_aligned_data_race_val<mefdn::uint64_t>(&other_pub[i]));
                w.write("my_priv_8b:0x{:x}\t"  , get_aligned_data_race_val<mefdn::uint64_t>(&my_priv[i]));
                w.write("my_pub_8b:0x{:x}\t"   , get_aligned_data_race_val<mefdn::uint64_t>(&my_pub[i]));
                
                const auto s = w.str();
                MEFDN_LOG_FATAL("{}", s);
                throw std::runtime_error(s);
            }
        }
        
        // Lost the exact position...
        fmt::MemoryWriter w;
        w.write("msg:Detected data race? (but lost exact position...)\t");
        w.write("blk_pos:{}\t", blk_pos);
        w.write("index:{}\t", index);
        w.write("other_pub:0x{:x}\t", reinterpret_cast<mefdn::intptr_t>(other_pub));
        w.write("my_priv:0x{:x}\t"  , reinterpret_cast<mefdn::intptr_t>(my_priv));
        w.write("my_pub:0x{:x}"     , reinterpret_cast<mefdn::intptr_t>(my_pub));
        
        const auto s = w.str();
        MEFDN_LOG_FATAL("{}", s);
        throw std::logic_error(s);
    }
    
    template <typename T>
    static T get_aligned_data_race_val(const mefdn::byte* const p) {
        auto ret_pi = reinterpret_cast<mefdn::uintptr_t>(p);
        
        // Align the pointer.
        ret_pi -= ret_pi % sizeof(T);
        
        return *reinterpret_cast<T*>(ret_pi);
    }
    
    typename rma_itf_type::template local_ptr<mefdn::byte>
    get_my_priv_ptr(const blk_pos_type blk_pos) {
        auto& self = this->derived();
        const auto blk_size = self.get_blk_size();
        return this->priv_ptr_ + (blk_size * blk_pos);
    }
    typename rma_itf_type::template local_ptr<mefdn::byte>
    get_my_pub_ptr(const blk_pos_type blk_pos) const noexcept {
        auto& self = this->derived();
        const auto blk_size = self.get_blk_size();
        return this->pub_buf_.local(blk_size * blk_pos);
    }
    
    #ifdef MEDSM2_ENABLE_LAZY_MERGE
    typename rma_itf_type::template remote_ptr<mefdn::byte>
    get_other_priv_ptr(const proc_id_type proc, const blk_pos_type blk_pos) {
        auto& self = this->derived();
        const auto blk_size = self.get_blk_size();
        return this->priv_buf_.remote(proc, blk_size * blk_pos);
    }
    #endif
    typename rma_itf_type::template remote_ptr<mefdn::byte>
    get_other_pub_ptr(const proc_id_type proc, const blk_pos_type blk_pos) {
        auto& self = this->derived();
        const auto blk_size = self.get_blk_size();
        return this->pub_buf_.remote(proc, blk_size * blk_pos);
    }
    
    typename rma_itf_type::template local_ptr<mefdn::byte>
        priv_ptr_;
    
    #ifdef MEDSM2_ENABLE_LAZY_MERGE
    typename P::template alltoall_ptr_set<mefdn::byte>
        priv_buf_;
    #endif
    
    typename rma_itf_type::template public_ptr<mefdn::byte>
        pub_ptr_;
    
    typename P::template alltoall_ptr_set<mefdn::byte>
        pub_buf_;
};

} // namespace medsm2
} // namespace menps

