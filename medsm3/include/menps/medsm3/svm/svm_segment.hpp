
#pragma once

#include <menps/medsm3/common.hpp>
#include <menps/mefdn/arithmetic.hpp>

#include <unistd.h>
#include <sys/mman.h>

namespace menps {
namespace medsm3 {

template <typename P>
class svm_segment
    : public P::segment_base_type
    // ts_segment or dir_segment
{
    using base = typename P::segment_base_type;

    using mapped_memory_type = typename P::mapped_memory_type;

    using com_itf_type = typename P::com_itf_type;
    using rma_itf_type = typename com_itf_type::rma_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    template <typename T>
    using local_ptr = typename rma_itf_type::template local_ptr<T>;
    template <typename T>
    using remote_ptr = typename rma_itf_type::template remote_ptr<T>;
    template <typename T>
    using alltoall_ptr_set = typename com_itf_type::template alltoall_ptr_set_t<T>;

    using byte = fdn::byte;

    using blk_subindex_type = typename P::blk_subindex_type;

public:
    template <typename Conf>
    explicit svm_segment(const Conf& conf)
        : base{conf.com, conf.num_blks}
        , rma_(conf.com.get_rma())
        , blk_size_{conf.blk_size}
    {
        this->ln2_blk_size_ =
                fdn::is_power_of_2(this->blk_size_)
            ?   fdn::floor_log2(this->blk_size_)
            :   0;
        
        byte* const working_app_ptr = conf.working_app_ptr;
        byte* const working_sys_ptr = conf.working_sys_ptr;
        byte* const primary_ptr = conf.primary_ptr;
        byte* const secondary_ptr = conf.secondary_ptr;
        const auto num_bytes = this->blk_size_ * this->get_num_blks();

        const auto base_flags = MAP_FIXED
            #ifdef MEDSM2_ENABLE_MAP_POPULATE
            | MAP_POPULATE
            #endif
            ;

        const auto shared_flags = base_flags | MAP_SHARED;
        const auto private_flags = base_flags | MAP_PRIVATE | MAP_ANONYMOUS;

        const bool is_migration_enabled = conf.is_migration_enabled;
       
        this->working_sys_map_ =
            mapped_memory_type::map(
                working_sys_ptr
            ,   num_bytes
            ,   PROT_READ | PROT_WRITE
            ,   shared_flags
            ,   conf.fd
            ,   reinterpret_cast<fdn::size_t>(working_app_ptr) // index_in_file
            );
        
        this->primary_map_ =
            mapped_memory_type::map(
                primary_ptr
            ,   num_bytes
            ,   PROT_READ | PROT_WRITE
            ,   private_flags
            ,   -1
            ,   0
            );

        if (!is_migration_enabled) {
            this->secondary_map_ =
                mapped_memory_type::map(
                    secondary_ptr
                ,   num_bytes
                ,   PROT_READ | PROT_WRITE
                ,   private_flags
                ,   -1
                ,   0
                );
        }
        
        if (conf.is_copied) {
            // This is used for initializing global variables.
            
            CMPTH_P_LOG_INFO(P
            ,   "Start copying global variables."
            ,   "working_sys_ptr", working_sys_ptr
            ,   "working_app_ptr", working_app_ptr
            ,   "primary_ptr", primary_ptr
            ,   "num_bytes", num_bytes
            );
            
            // Initialize the private area with the original contents.
            copy_memory(working_sys_ptr, working_app_ptr, num_bytes);
            // Initialize the public area with the original contents.
            copy_memory(primary_ptr, working_app_ptr, num_bytes);
            if (!is_migration_enabled) {
                copy_memory(secondary_ptr, working_app_ptr, num_bytes);
            }
            
            CMPTH_P_LOG_INFO(P, "Finish copying global variables.");
        }
        else {
            // TODO: May be unnecessary
            // Initialize all the contents.
            fill_zero(working_sys_ptr, num_bytes);
            fill_zero(primary_ptr, num_bytes);
            if (!is_migration_enabled) {
                fill_zero(secondary_ptr, num_bytes);
            }
        }
        
        // The mapping for the application working buffer must be created at last
        // for copying the initial states of global variables.
        this->working_app_map_ =
            mapped_memory_type::map(
                working_app_ptr
            ,   num_bytes
            ,   PROT_NONE
            ,   shared_flags
            ,   conf.fd
            ,   reinterpret_cast<fdn::size_t>(working_app_ptr) // index_in_file
            );

        auto& com = conf.com;
        auto& rma = com.get_rma();
        auto& coll = com.get_coll();
        
        // Attach the buffers.
        const auto working_lptr = rma.attach(working_sys_ptr, working_sys_ptr + num_bytes);
        const auto primary_lptr = rma.attach(primary_ptr, primary_ptr + num_bytes);
        
        this->working_buf_.coll_make(rma, coll, working_lptr, num_bytes);
        this->primary_buf_.coll_make(rma, coll, primary_lptr, num_bytes);
    }

    ~svm_segment()
    {
        this->rma_.detach(this->working_buf_.local(0));
        this->rma_.detach(this->primary_buf_.local(0));
    }
    
    byte* get_local_working_app_ptr() {
        return this->get_local_working_app_ptr(0);
    }
    byte* get_local_working_app_ptr(const blk_subindex_type blk_sidx) {
        CMPTH_P_ASSERT(P, this->is_valid_subindex(blk_sidx));
        const auto working_app_ptr = static_cast<byte*>(this->working_app_map_.get());
        return working_app_ptr + blk_sidx * this->get_blk_size();
    }
    local_ptr<byte> get_local_working_lptr(const blk_subindex_type blk_sidx) {
        CMPTH_P_ASSERT(P, this->is_valid_subindex(blk_sidx));
        return this->working_buf_.local(blk_sidx * this->get_blk_size());
    }
    remote_ptr<byte> get_remote_working_rptr(
        const proc_id_type      proc
    ,   const blk_subindex_type blk_sidx
    ) {
        CMPTH_P_ASSERT(P, this->is_valid_subindex(blk_sidx));
        return this->working_buf_.remote(proc, blk_sidx * this->get_blk_size());
    }
    local_ptr<byte> get_local_primary_lptr(const blk_subindex_type blk_sidx) {
        CMPTH_P_ASSERT(P, this->is_valid_subindex(blk_sidx));
        return this->primary_buf_.local(blk_sidx * this->get_blk_size());
    }
    remote_ptr<byte> get_remote_primary_rptr(
        const proc_id_type      proc
    ,   const blk_subindex_type blk_sidx
    ) {
        CMPTH_P_ASSERT(P, this->is_valid_subindex(blk_sidx));
        return this->primary_buf_.remote(proc, blk_sidx * this->get_blk_size());
    }
    byte* get_local_secondary_ptr(const blk_subindex_type blk_sidx) {
        CMPTH_P_ASSERT(P, this->is_valid_subindex(blk_sidx));
        const auto secondary_ptr = static_cast<byte*>(this->secondary_map_.get());
        CMPTH_P_ASSERT(P, secondary_ptr != nullptr);
        return &secondary_ptr[blk_sidx * this->get_blk_size()];
    }

    fdn::size_t get_blk_size() const noexcept {
        return this->blk_size_;
    }
    bool try_get_subindex_from_app_ptr(const void* const ptr, blk_subindex_type* const out_sidx) {
        const auto iptr = reinterpret_cast<fdn::uintptr_t>(ptr);

        const auto working_app_start = reinterpret_cast<fdn::uintptr_t>(this->working_app_map_.get());
        const auto working_app_end = working_app_start + (this->get_num_blks() * this->get_blk_size());

        if (working_app_start <= iptr && iptr < working_app_end) {
            const auto ln2_blk_size = this->ln2_blk_size_;
            *out_sidx = 
                    ln2_blk_size > 0
                ?   ((iptr - working_app_start) >> ln2_blk_size)
                :   ((iptr - working_app_start) / this->get_blk_size());
            return true;
        }
        else {
            return false;
        }
    }

    int get_tag_from_subindex(const blk_subindex_type blk_sidx) const noexcept {
        CMPTH_P_ASSERT(P, this->is_valid_subindex(blk_sidx));
        const auto working_app_start = reinterpret_cast<fdn::uintptr_t>(this->working_app_map_.get());
        const auto blk_working_app = working_app_start + blk_sidx * this->get_blk_size();
        return static_cast<int>(blk_working_app >> 12); // TODO: Magic number (12 = log2(PAGE_SIZE))
    }

private:
    static void copy_memory(void* const dest, const void* const src, const fdn::size_t num_bytes)
    {
        CMPTH_P_LOG_DEBUG(P
        ,   "Start copying memory."
        ,   "dest", dest
        ,   "src", src
        ,   "num_bytes", num_bytes
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
        
        for (fdn::size_t i = 0; i < num_bytes; ++i) {
            dest_byte[i] = src_byte[i];
        }
        #endif
        
        CMPTH_P_LOG_DEBUG(P
        ,   "Finish copying memory."
        ,   "dest", dest
        ,   "src", src
        ,   "num_bytes", num_bytes
        );
    }

    static void fill_zero(void* const dest, const fdn::size_t num_bytes)
    {
        CMPTH_P_LOG_DEBUG(P
        ,   "Start filling with zero."
        ,   "dest", dest
        ,   "num_bytes", num_bytes
        );
        
        #if 1
        memset(dest, 0, num_bytes);
        #else
        const auto dest_byte = static_cast<fdn::byte*>(dest);
        for (size_type i = 0; i < num_bytes; ++i) {
            dest_byte[i] = static_cast<fdn::byte>(0);
        }
        #endif
        
        CMPTH_P_LOG_DEBUG(P
        ,   "Finish filling with zero."
        ,   "dest", dest
        ,   "num_bytes", num_bytes
        );
    }

    rma_itf_type& rma_;

    fdn::size_t blk_size_ = 0;
    fdn::size_t ln2_blk_size_ = 0;

    mapped_memory_type  working_app_map_;
    mapped_memory_type  working_sys_map_;
    mapped_memory_type  primary_map_;
    mapped_memory_type  secondary_map_;
    
    alltoall_ptr_set<byte> working_buf_;
    alltoall_ptr_set<byte> primary_buf_;
};

} // namespace medsm3
} // namespace menps

