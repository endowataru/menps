
#pragma once

#include <menps/medsm2/dsm/blk_lock_table.hpp>
#include <menps/medsm2/dsm/blk_data_table.hpp>
#include <menps/medsm2/dsm/blk_dir_table.hpp>
#ifdef MEDSM2_USE_SIG_BUFFER_MERGE_TABLE
    #include <menps/medsm2/dsm/blk_flag_table.hpp>
#endif
#include <menps/mefdn/memory/mapped_memory.hpp>
#include <unistd.h>
#include <sys/mman.h>

namespace menps {
namespace medsm2 {

template <typename P>
class svm_blk_table;

template <typename P>
struct svm_blk_table_policy : P
{
    using derived_type = svm_blk_table<P>;
};

template <typename P>
class svm_blk_table
    : private blk_lock_table<svm_blk_table_policy<P>>
    // TODO: compilation error; simplify the class hierarchy
    , public blk_dir_table<svm_blk_table_policy<P>>
    , private blk_data_table<svm_blk_table_policy<P>>
{
    using blk_pos_type = typename P::blk_pos_type;
    using blk_id_type = typename P::blk_id_type;
    using size_type = typename P::size_type;
    
    using com_itf_type = typename P::com_itf_type;
    using coll_itf_type = typename com_itf_type::coll_itf_type;
    
    using base_policy_type = svm_blk_table_policy<P>;
    
    using ptrdiff_type = typename P::ptrdiff_type;
    
public:
    using lock_table_type = blk_lock_table<base_policy_type>;
    using dir_table_type = blk_dir_table<base_policy_type>;
    using data_table_type = blk_data_table<base_policy_type>;
    #ifdef MEDSM2_USE_SIG_BUFFER_MERGE_TABLE
    using flag_table_type = blk_flag_table<base_policy_type>;
    #endif
    
    #ifdef MEDSM2_ENABLE_FAST_RELEASE
    // TODO: simplify the class hierarchy
    using lock_table_type::read_lock_entry;
    using lock_table_type::write_lock_entry;
    #endif
    
    template <typename Conf>
    void coll_make(const Conf& conf)
    {
        this->blk_size_ = conf.blk_size;
        this->num_blks_ = conf.num_blks;
        
        this->ln2_blk_size_ =
                mefdn::is_power_of_2(this->blk_size_)
            ?   mefdn::floor_log2(this->blk_size_)
            :   0;
        
        const auto priv_app_ptr = conf.priv_app_ptr;
        const auto priv_sys_ptr = conf.priv_sys_ptr;
        const auto pub_ptr = conf.pub_ptr;
        const auto num_bytes = this->blk_size_ * this->num_blks_;
       
        this->priv_sys_map_ =
            mefdn::mapped_memory::map(
                priv_sys_ptr
            ,   num_bytes
            ,   PROT_READ | PROT_WRITE
            ,   MAP_FIXED | MAP_SHARED
                #ifdef MEDSM2_ENABLE_MAP_POPULATE
                | MAP_POPULATE
                #endif
            ,   conf.fd
            ,   reinterpret_cast<mefdn::size_t>(priv_app_ptr) // index_in_file
            );
        
        this->pub_map_ =
            mefdn::mapped_memory::map(
                pub_ptr
            ,   num_bytes
            ,   PROT_READ | PROT_WRITE
            ,   MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS
                #ifdef MEDSM2_ENABLE_MAP_POPULATE
                | MAP_POPULATE
                #endif
            ,   -1
            ,   0
            );

        #ifndef MEDSM2_ENABLE_MIGRATION
        const auto snapshot_ptr = conf.snapshot_ptr;
        this->snapshot_map_ =
            mefdn::mapped_memory::map(
                snapshot_ptr
            ,   num_bytes
            ,   PROT_READ | PROT_WRITE
            ,   MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS
                #ifdef MEDSM2_ENABLE_MAP_POPULATE
                | MAP_POPULATE
                #endif
            ,   -1
            ,   0
            );
        #endif
        
        if (conf.is_copied) {
            // This is used for initializing global variables.
            
            MEFDN_LOG_DEBUG(
                "msg:Start copying global variables.\t"
                "priv_sys_ptr:{:x}\t"
                "priv_app_ptr:{:x}\t"
                "pub_ptr:{:x}\t"
                "num_bytes:{:x}"
            ,   reinterpret_cast<mefdn::uintptr_t>(priv_sys_ptr)
            ,   reinterpret_cast<mefdn::uintptr_t>(priv_app_ptr)
            ,   reinterpret_cast<mefdn::uintptr_t>(pub_ptr)
            ,   num_bytes
            );
            
            // Initialize the private area with the original contents.
            copy_memory(priv_sys_ptr, priv_app_ptr, num_bytes);
            // Initialize the public area with the original contents.
            copy_memory(pub_ptr, priv_app_ptr, num_bytes);
            #ifndef MEDSM2_ENABLE_MIGRATION
            copy_memory(snapshot_ptr, priv_app_ptr, num_bytes);
            #endif
            
            MEFDN_LOG_DEBUG(
                "msg:Finish copying global variables."
            );
        }
        else {
            // TODO: May be unnecessary
            // Initialize all the contents.
            fill_zero(priv_sys_ptr, num_bytes);
            fill_zero(pub_ptr, num_bytes);
            #ifndef MEDSM2_ENABLE_MIGRATION
            fill_zero(snapshot_ptr, num_bytes);
            #endif
        }
        
        this->priv_app_map_ =
            mefdn::mapped_memory::map(
                priv_app_ptr
            ,   num_bytes
            ,   PROT_NONE
            ,   MAP_FIXED | MAP_SHARED
                #ifdef MEDSM2_ENABLE_MAP_POPULATE
                | MAP_POPULATE
                #endif
            ,   conf.fd
            ,   reinterpret_cast<mefdn::size_t>(priv_app_ptr) // index_in_file
            );
        
        struct dir_tbl_conf {
            com_itf_type&   com;
            size_type       num_blks;
        };
        
        const dir_tbl_conf dir_conf{ conf.com, this->num_blks_ };
        
        lock_table_type::coll_make(dir_conf);
        
        dir_table_type::coll_make(dir_conf);
        
        struct data_tbl_conf {
            com_itf_type&   com;
            size_type       seg_size;
            void*           priv_buf;
            void*           pub_buf;
            #ifndef MEDSM2_ENABLE_MIGRATION
            void*           snapshot_buf;
            #endif
        };
        
        data_table_type::coll_make(
            data_tbl_conf{ conf.com, num_bytes, priv_sys_ptr, pub_ptr
            #ifndef MEDSM2_ENABLE_MIGRATION
            , snapshot_ptr
            #endif
            }
        );
        
        #ifdef MEDSM2_USE_SIG_BUFFER_MERGE_TABLE
        this->flag_tbl_.init(this->num_blks_);
        #endif
    }
    
private:
    static void copy_memory(void* const dest, const void* const src, const size_type num_bytes)
    {
        MEFDN_LOG_DEBUG(
            "msg:Start copying memory.\t"
            "dest:0x{:x}\t"
            "src:0x{:x}\t"
            "num_bytes:0x{:x}"
        ,   reinterpret_cast<mefdn::uintptr_t>(dest)
        ,   reinterpret_cast<mefdn::uintptr_t>(src)
        ,   num_bytes
        );
        
        #ifdef MEDSM2_ENABLE_MAP_POPULATE
        // Simply call memcpy if MAP_POPULATE is enabled.
        std::memcpy(dest, src, num_bytes);
        
        #else
        // Note: glibc's memcpy processes the memory backward.
        //       If MAP_POPULATE is not added to the mmap argument,
        //       memcpy() becomes really slower than the code below
        //       due to the implementation of Linux page tables.
        //       Thanks to Mr. Hiroki Nakazawa for solving this issue.
        
        const auto dest_byte = static_cast<mefdn::byte*>(dest);
        const auto src_byte = static_cast<const mefdn::byte*>(src);
        
        for (size_type i = 0; i < num_bytes; ++i) {
            dest_byte[i] = src_byte[i];
        }
        #endif
        
        MEFDN_LOG_DEBUG(
            "msg:Finish copying memory.\t"
            "dest:0x{:x}\t"
            "src:0x{:x}\t"
            "num_bytes:0x{:x}"
        ,   reinterpret_cast<mefdn::uintptr_t>(dest)
        ,   reinterpret_cast<mefdn::uintptr_t>(src)
        ,   num_bytes
        );
    }
    
    static void fill_zero(void* const dest, const size_type num_bytes)
    {
        MEFDN_LOG_DEBUG(
            "msg:Start filling with zero.\t"
            "dest:0x{:x}\t"
            "num_bytes:0x{:x}"
        ,   reinterpret_cast<mefdn::uintptr_t>(dest)
        ,   num_bytes
        );
        
        #if 1
        memset(dest, 0, num_bytes);
        #else
        const auto dest_byte = static_cast<mefdn::byte*>(dest);
        for (size_type i = 0; i < num_bytes; ++i) {
            dest_byte[i] = static_cast<mefdn::byte>(0);
        }
        #endif
        
        MEFDN_LOG_DEBUG(
            "msg:Finish filling with zero.\t"
            "dest:0x{:x}\t"
            "num_bytes:0x{:x}"
        ,   reinterpret_cast<mefdn::uintptr_t>(dest)
        ,   num_bytes
        );
    }
    
public:
    using data_table_type::finalize; // TODO: finalize dir_table_type too
    
    using dir_table_type::check_locked;
    
    size_type get_blk_size() const noexcept {
        return this->blk_size_;
    }
    blk_pos_type get_blk_pos_from_blk_id(const blk_id_type blk_id) {
        // The block ID can be converted to a pointer.
        // TODO: This relationship may be changed in the future.
        const auto ptr =
            reinterpret_cast<mefdn::byte*>(blk_id);
        
        const auto priv_app_ptr =
            static_cast<mefdn::byte*>(this->priv_app_map_.get());
        
        const auto diff = ptr - priv_app_ptr;
        
        // TODO: Remove this division operation.
        const auto ln2_blk_size = this->ln2_blk_size_;
        if (MEFDN_LIKELY(ln2_blk_size > 0)) {
            const auto ret = diff >> ln2_blk_size;
            MEFDN_ASSERT(ret == static_cast<ptrdiff_type>(diff / blk_size_));
            return ret;
        }
        else {
            return diff / blk_size_;
        }
    }
    
    lock_table_type& get_lock_tbl() { return *this; }
    dir_table_type& get_dir_tbl() { return *this; }
    data_table_type& get_data_tbl() { return *this; }
    #ifdef MEDSM2_USE_SIG_BUFFER_MERGE_TABLE
    flag_table_type& get_flag_tbl() { return this->flag_tbl_; }
    #endif
    
private:
    friend lock_table_type;
    friend data_table_type;
    
    void set_inaccessible(const blk_pos_type blk_pos, const size_type num_bytes) {
        this->call_mprotect(blk_pos, num_bytes, PROT_NONE);
    }
    void set_readonly(const blk_pos_type blk_pos, const size_type num_bytes) {
        this->call_mprotect(blk_pos, num_bytes, PROT_READ);
    }
    void set_writable(const blk_pos_type blk_pos, const size_type num_bytes) {
        this->call_mprotect(blk_pos, num_bytes, PROT_READ | PROT_WRITE);
    }
    
    void call_mprotect(const blk_pos_type blk_pos, const size_type num_bytes, const int prot)
    {
        const auto priv_app_ptr =
            static_cast<mefdn::byte*>(this->priv_app_map_.get());
        
        const auto p = &priv_app_ptr[blk_pos * blk_size_];
        
        const auto ret = mprotect(p, num_bytes, prot);
        
        if (ret == 0) {
            MEFDN_LOG_INFO(
                "msg:Called mprotect().\t"
                "ptr:{:x}\t"
                "num_bytes:{}\t"
                "prot:{}\t"
            ,   reinterpret_cast<mefdn::uintptr_t>(p)
            ,   num_bytes
            ,   prot
            );
        }
        else {
            MEFDN_LOG_FATAL(
                "msg:mprotect() failed.\t"
                "ptr:{:x}\t"
                "num_bytes:{}\n"
                "prot:{}\t"
                "ret:{}\t"
                "errno:{}"
            ,   reinterpret_cast<mefdn::uintptr_t>(p)
            ,   num_bytes
            ,   prot
            ,   ret
            ,   errno
            );
            
            throw std::exception{};
        }
    }
    
    size_type       blk_size_;
    size_type       num_blks_;
    size_type       ln2_blk_size_;
    
    mefdn::mapped_memory   pub_map_;
    mefdn::mapped_memory   priv_sys_map_;
    mefdn::mapped_memory   priv_app_map_;
    #ifndef MEDSM2_ENABLE_MIGRATION
    mefdn::mapped_memory   snapshot_map_;
    #endif
    
    #ifdef MEDSM2_USE_SIG_BUFFER_MERGE_TABLE
    flag_table_type flag_tbl_;
    #endif
};

} // namespace medsm2
} // namespace menps

