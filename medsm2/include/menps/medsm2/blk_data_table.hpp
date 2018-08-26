
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/logger.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <cstring>

namespace menps {
namespace medsm2 {

#define MEDSM2_USE_COMPARE_DIFF
//#define MEDSM2_DISABLE_READ_MERGE
//#define MEDSM2_USE_SIMD_DIFF
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
        // TODO: This buffer only works as a local buffer of RDMA.
        this->priv_ptr_ = conf.com.get_rma().attach(priv, priv + conf.seg_size);
        
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
                // Merge the diff in the read.
                
                // Read the public data of the home process into a temporary buffer.
                const auto home_pub_buf =
                    rma.buf_read(
                        home_proc
                    ,   this->get_other_pub_ptr(home_proc, blk_pos)
                    ,   blk_size
                    );
                
                const auto home_pub = home_pub_buf.get();
                
                const auto my_pub = this->get_my_pub_ptr(blk_pos);
                
                // Apply the changes written in home_pub into my_priv and my_pub.
                this->read_merge(blk_pos, home_pub, my_priv, my_pub, blk_size);
            }
            else {
            #endif
                // Simply read from the home process.
                rma.read(home_proc, this->get_other_pub_ptr(home_proc, blk_pos), my_priv, blk_size);
            #ifndef MEDSM2_DISABLE_READ_MERGE
            }
            #endif
        }
        
        // Call mprotect(PROT_READ).
        self.set_readonly(blk_pos, blk_size);
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
            // Copy the private data to the public data.
            // This is a preparation for releasing this block later.
            std::memcpy(my_pub, my_priv, blk_size);
            //std::copy(my_priv, my_priv + blk_size, my_pub);
        }
        
        // Call mprotect(PROT_READ | PROT_WRITE).
        self.set_writable(blk_pos, blk_size);
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
            
            // Call mprotect(PROT_READ).
            self.set_readonly(blk_pos, blk_size);
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
            #ifdef MEDSM2_ENABLE_NEEDS_LOCAL_COMP
            if (glk_ret.needs_local_comp){
                is_written = std::memcmp(my_priv, my_pub, blk_size) != 0;
            }
            else {
                is_written = true;
            }
            #else
            is_written = std::memcmp(my_priv, my_pub, blk_size) != 0;
            #endif
                //std::equal(my_priv, my_priv + blk_size, my_pub)
        }
        
        if (! glk_ret.is_remotely_updated) {
            #ifndef MEDSM2_FORCE_ALWAYS_MERGE_LOCAL
            if (is_written) {
            #endif
                // Copy the private data to the public data.
                std::memcpy(my_pub, my_priv, blk_size);
                //std::copy(my_priv, my_priv + blk_size, my_pub);
            #ifndef MEDSM2_FORCE_ALWAYS_MERGE_LOCAL
            }
            else {
                // The data is not modified.
            }
            #endif
        }
        else {
            const auto cur_owner = glk_ret.owner;
            
            // Read the public data from cur_owner.
            const auto other_pub_buf =
                rma.buf_read(
                    cur_owner
                ,   this->get_other_pub_ptr(cur_owner, blk_pos)
                ,   blk_size
                );
            
            const auto other_pub = other_pub_buf.get();
            
            #ifndef MEDSM2_FORCE_ALWAYS_MERGE_REMOTE
            if (is_written) {
            #endif
                // Three copies (my_pub, my_priv, other_pub) are different with each other.
                // It is necessary to merge them to complete the release.
                this->write_merge(blk_pos, other_pub, my_priv, my_pub, blk_size);
                
            #ifndef MEDSM2_FORCE_ALWAYS_MERGE_REMOTE
            }
            else {
                // Although this process doesn't release this block at this time,
                // the buffer read from the current owner can be utilized.
                // This is important when an acquire on this block is on-going
                // because that thread requires this releaser thread
                // to make the latest modifications visible on this process.
                // Note: The timestamp should also be updated in the directory later.
                std::memcpy(my_priv, other_pub, blk_size);
                //std::copy(other_pub, other_pub + blk_size, my_priv);
                std::memcpy(my_pub , other_pub, blk_size);
                //std::copy(other_pub, other_pub + blk_size, my_pub);
            }
            #endif
        }
        
        if (glk_ret.needs_protect_after) {
            // If this block was inaccessible (invalid-clean or invalid-dirty) from the application,
            // make the block readable now.
            
            // Call mprotect(PROT_READ).
            self.set_readonly(blk_pos, blk_size);
        }
        
        return { is_written };
        
        // The temporary buffer is discarded in its destructor here.
    }
    
    void invalidate(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        const auto blk_size = self.get_blk_size();
        
        // Call mprotect(PROT_NONE).
        self.set_inaccessible(blk_pos, blk_size);
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
    ,   const mefdn::byte* const    home_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ) {
        for (size_type i = 0; i < blk_size; ++i) {
            #ifdef MEDSM2_USE_COMPARE_DIFF
            if (my_priv[i] != my_pub[i]) {
                if (my_pub[i] != home_pub[i]) {
                    report_data_race(blk_pos, i, home_pub, my_priv, my_pub, blk_size);
                }
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
    
    static void write_merge(
        const blk_pos_type          blk_pos
    ,   const mefdn::byte* const    other_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ) {
        #ifdef MEDSM2_USE_SIMD_DIFF
        #define VECTOR_LEN  32
        using vec_buf_type = unsigned char __attribute__((vector_size(VECTOR_LEN)));
        
        MEFDN_ASSERT(blk_size % VECTOR_LEN == 0);
        
        //__asm__ __volatile__ ("# merge starts");
        
        for (size_type i = 0; i < blk_size; i += VECTOR_LEN) {
            auto* other_pub_vec = reinterpret_cast<const vec_buf_type*>(&other_pub[i]);
            auto* my_priv_vec   = reinterpret_cast<vec_buf_type*>(&my_priv[i]);
            auto* my_pub_vec    = reinterpret_cast<vec_buf_type*>(&my_pub[i]);
            
            const auto other_pub_val = *other_pub_vec;
            const auto my_priv_val   = *my_priv_vec;
            const auto my_pub_val    = *my_pub_vec;
            
            const auto other_diff = other_pub_val ^ my_pub_val;
            const auto my_diff    = my_priv_val   ^ my_pub_val;
            
            const auto result = my_diff ^ other_pub_val;
            #if 0
            const auto racy = other_diff & my_diff;
            
            if (racy != zero) {
                report_data_race(blk_pos, other_pub, my_priv, my_pub, blk_size);
            }
            #endif
            
            *my_priv_vec = result;
            *my_pub_vec = result;
        }
        //__asm__ __volatile__ ("# merge ends");
        
        #else
        for (size_type i = 0; i < blk_size; ++i) {
            #ifdef MEDSM2_USE_COMPARE_DIFF
            if (my_priv[i] != my_pub[i]) {
                if (my_pub[i] != other_pub[i]) {
                    report_data_race(blk_pos, i, other_pub, my_priv, my_pub, blk_size);
                }
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
        #endif
    }
    
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
    
    typename rma_itf_type::template remote_ptr<mefdn::byte>
    get_other_pub_ptr(const proc_id_type proc, const blk_pos_type blk_pos) {
        auto& self = this->derived();
        const auto blk_size = self.get_blk_size();
        return this->pub_buf_.remote(proc, blk_size * blk_pos);
    }
    
    typename rma_itf_type::template local_ptr<mefdn::byte>
        priv_ptr_;
    
    typename P::template alltoall_ptr_set<mefdn::byte>
        pub_buf_;
    
    typename rma_itf_type::template public_ptr<mefdn::byte>
        pub_ptr_;
};

} // namespace medsm2
} // namespace menps

