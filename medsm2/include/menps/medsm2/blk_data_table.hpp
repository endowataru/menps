
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace medsm2 {

#define MEDSM2_USE_COMPARE_DIFF
//#define MEDSM2_AVOID_DOWNGRADE

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
        this->priv_buf_ = static_cast<mefdn::byte*>(conf.priv_buf);
        
        const auto pub = static_cast<mefdn::byte*>(conf.pub_buf);
        
        // Attach the public buffer.
        conf.com.get_rma().attach(pub, pub + conf.seg_size);
        
        this->pub_buf_.coll_make(conf.com.get_rma(), conf.com.get_coll(), pub, conf.seg_size);
    }
    
    void finalize(com_itf_type& com)
    {
        void* const pub = pub_buf_.local(0);
        
        // Detach the public buffer.
        com.get_rma().detach(pub);
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
            if (is_dirty) {
                // Merge the diff in the read.
                
                const auto home_pub_buf =
                    rma.template make_unique<mefdn::byte []>(blk_size);
                
                const auto home_pub = home_pub_buf.get();
                
                // Read the public data of the home process into a temporary buffer.
                rma.read(
                    home_proc
                ,   this->get_other_pub_ptr(home_proc, blk_pos)
                ,   home_pub_buf.get()
                ,   blk_size
                );
                
                const auto my_pub = this->get_my_pub_ptr(blk_pos);
                
                // TODO: Optimize.
                for (size_type i = 0; i < blk_size; ++i) {
                    #ifdef MEDSM2_USE_COMPARE_DIFF
                    if (my_priv[i] != my_pub[i]) {
                        if (my_pub[i] != home_pub[i]) {
                            throw data_race_error(
                                fmt::format(
                                    "Data race in read ("
                                    "home_pub:{}, my_pub:{}, my_priv:{})"
                                ,   static_cast<unsigned char>(home_pub[i])
                                ,   static_cast<unsigned char>(my_pub[i])
                                ,   static_cast<unsigned char>(my_priv[i])
                                )
                            );
                        }
                        my_priv[i] = my_pub[i];
                    }
                    else {
                        my_pub[i] = home_pub[i];
                        my_priv[i] = home_pub[i];
                    }
                    
                    #else
                    // TODO: Scoped enums cannot do XOR...
                    my_priv[i] =
                        static_cast<mefdn::byte>(
                            static_cast<unsigned char>(my_priv[i]) ^
                            static_cast<unsigned char>(my_pub[i]) ^
                            static_cast<unsigned char>(home_pub[i])
                        );
                    #endif
                }
            }
            else {
                // Simply read from the home process.
                rma.read(home_proc, this->get_other_pub_ptr(home_proc, blk_pos), my_priv, blk_size);
            }
        }
        
        // Call mprotect(PROT_READ).
        self.set_readonly(blk_pos, blk_size);
    }
    
    void start_write(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto blk_size = self.get_blk_size();
        
        const auto my_priv = this->get_my_priv_ptr(blk_pos);
        const auto my_pub = this->get_my_pub_ptr(blk_pos);
        
        // Copy the private data to the public data.
        // This is a preparation for releasing this block later.
        std::copy(my_priv, my_priv + blk_size, my_pub);
        
        // Call mprotect(PROT_READ | PROT_WRITE).
        self.set_writable(blk_pos, blk_size);
    }
    
    struct merge_to_result {
        // This block was migrated from the old owner
        // to the current process.
        bool is_migrated;
        // This block was written
        // and must be recorded to the write notices.
        bool is_written;
        // This block became inaccessible (PROT_NONE).
        bool is_downgraded;
    };
    
    merge_to_result merge_to_public(
        com_itf_type&           com
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const proc_id_type      cur_owner
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto blk_size = self.get_blk_size();
        const auto cur_proc = com.this_proc_id();
        auto& rma = com.get_rma();
        
        const auto my_priv = this->get_my_priv_ptr(blk_pos);
        const auto my_pub = this->get_my_pub_ptr(blk_pos);
        
        merge_to_result r{};
        
        #ifdef MEDSM2_AVOID_DOWNGRADE
        if (cur_proc == cur_owner) {
            // Compare the private data with the public data.
            // Note that the private data is STILL WRITABLE
            // and can be modified concurrently by other threads in this process.
            // It's OK to read the intermediate states
            // because those writes will be managed by the next release operation.
            if (std::equal(my_priv, my_priv + blk_size, my_pub)) {
                // The data is not modified.
                r = merge_to_result{ false, false, false };
            }
            else {
                // Copy to the private data.
                std::copy(my_priv, my_priv + blk_size, my_pub);
                
                r = merge_to_result{ false, true, false };
            }
        }
        else {
        #endif
            // Create a temporary buffer.
            // TODO: Reuse this buffer.
            const auto other_pub_buf =
                rma.template make_unique_uninitialized<mefdn::byte []>(blk_size);
            
            const auto other_pub = other_pub_buf.get();
            
            // Read the public data from cur_owner.
            rma.read(cur_owner, this->get_other_pub_ptr(cur_owner, blk_pos), other_pub, blk_size);
            
            #ifdef MEDSM2_AVOID_DOWNGRADE
            // Compare the public data with that of cur_owner.
            if (std::equal(my_pub, my_pub + blk_size, other_pub)) {
                // The current owner has the same public data.
                // It means that the latest writer didn't update this block
                // after the current process started writing it.
                // Therefore, the current process can skip merging this block.
                
                // Just copy the private data to the public area.
                // Note that the private data can be concurrently written by the other threads.
                std::copy(my_priv, my_priv + blk_size, my_pub);
                
                r = merge_to_result{ true, true, false };
            }
            else {
            #endif
                // Call mprotect(PROT_READ).
                self.set_readonly(blk_pos, blk_size);
                
                // Compare the public data with the private data.
                if (std::equal(my_pub, my_pub + blk_size, my_priv)) {
                    // Although this process doesn't release this block at this time,
                    // the buffer read from the current owner can be utilized.
                    // This is important when an acquire on this block is on-going
                    // because that thread requires this releaser thread
                    // to make the latest modifications visible on this process.
                    // Note: The timestamp should also be updated in the directory later.
                    std::copy(other_pub, other_pub + blk_size, my_priv);
                    std::copy(other_pub, other_pub + blk_size, my_pub);
                    
                    // This block is not written by the current process.
                    // It means that releasing this block now is unnecessary.
                    r = merge_to_result{ false, false, true };
                }
                else {
                    // Three copies (my_pub, my_priv, other_pub) are different with each other.
                    // It is necessary to merge them to complete the release.
                    this->merge(other_pub, my_priv, my_pub, blk_size);
                    
                    r = merge_to_result{ true, true, true };
                }
            #ifdef MEDSM2_AVOID_DOWNGRADE
            }
        }
        #endif
        
        return r;
        
        // The temporary buffer is discarded in its destructor here.
    }
    
    #if 0
    void merge_from_public(
        com_itf_type&           com
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ,   const proc_id_type      home_proc
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto blk_size = self.get_blk_size();
        auto& rma = com.get_rma();
        
        const auto my_priv = this->get_my_priv_ptr(blk_pos);
        const auto my_pub = this->get_my_pub_ptr(blk_pos);
    
        // Create a temporary buffer.
        // TODO: Reuse this buffer.
        const auto home_pub_buf =
            rma.template make_unique<mefdn::byte []>(blk_size);
        
        const auto home_pub = home_pub_buf.get();
        
        // Read the public data from home_proc.
        rma.read(
            home_proc
        ,   this->get_other_pub_ptr(home_proc, blk_pos)
        ,   home_pub_buf.get()
        ,   blk_size
        );
        
        // TODO: Improve performance.
        for (size_type i = 0; i < blk_size; ++i) {
            // TODO: Scoped enums cannot do XOR...
            const auto x =
                static_cast<unsigned char>(my_pub[i]) ^
                static_cast<unsigned char>(home_pub[i]);
            
            if (x != 0) {
                my_priv[i] =
                    static_cast<mefdn::byte>(
                        static_cast<unsigned char>(my_priv[i]) ^ x
                    );
            }
            
            my_pub[i] = my_priv[i];
        }
    }
    #endif
    
    void invalidate(const blk_pos_type blk_pos, const unique_lock_type& lk)
    {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        const auto blk_size = self.get_blk_size();
        
        // Call mprotect(PROT_NONE).
        self.set_inaccessible(blk_pos, blk_size);
    }
    
private:
    static void merge(
        const mefdn::byte* const    other_pub
    ,         mefdn::byte* const    my_priv
    ,         mefdn::byte* const    my_pub
    ,   const size_type             blk_size
    ) {
        for (size_type i = 0; i < blk_size; ++i) {
            #ifdef MEDSM2_USE_COMPARE_DIFF
            if (my_priv[i] != my_pub[i]) {
                if (my_pub[i] != other_pub[i]) {
                    throw data_race_error(
                        fmt::format(
                            "Data race in release ("
                            "other_pub:{}, my_pub:{}, my_priv:{})"
                        ,   static_cast<unsigned char>(other_pub[i])
                        ,   static_cast<unsigned char>(my_pub[i])
                        ,   static_cast<unsigned char>(my_priv[i])
                        )
                    );
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
    }
    
    mefdn::byte* get_my_priv_ptr(const blk_pos_type blk_pos) {
        auto& self = this->derived();
        const auto blk_size = self.get_blk_size();
        return &this->priv_buf_[blk_size * blk_pos];
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
    
    mefdn::byte* priv_buf_ = nullptr;
    
    typename P::template alltoall_ptr_set<mefdn::byte>
        pub_buf_;
};

} // namespace medsm2
} // namespace menps

