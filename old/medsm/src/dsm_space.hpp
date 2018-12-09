
#include "manager/rpc_manager_segment_proxy.hpp"
#include "sharer/sharer_space_proxy.hpp"
#include "page_fault_upgrader.hpp"
#include <menps/medsm/space.hpp>
#include <string>
#include <menps/mefdn/external/fmt.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <menps/mefdn/logger.hpp>
#include "access_history.hpp"
#include "protector/protector_space.hpp"
#include "protector/protector_space_proxy.hpp"
#include "dsm_segment.hpp"
#include "shm_object.hpp"

#ifndef __APPLE__
    #define MEDSM_USE_GLOBAL_VAR
#endif

#ifdef MEDSM_USE_GLOBAL_VAR

extern "C" {

extern void* _dsm_data_begin;
extern void* _dsm_data_end;

} // extern "C"

#endif

namespace menps {
namespace medsm {

class dsm_space
    : public space
{
    struct manager_space_conf
    {
        mefdn::size_t      num_segments;
    };
    
    struct sharer_space_conf
    {
        mefdn::size_t      num_segments;
    };
    
    struct protector_space_conf
    {
        mefdn::size_t      num_segments;
        sharer_space&       sharer_sp;
        mefdn::size_t      max_seg_size;
        int                 fd;
    };
    
public:
    dsm_space()
        : shm_obj_(
            shm_object::config{
                get_reg_name().c_str()
            ,   get_address_space_size()
            }
        )
        , manager_(
            mefdn::make_unique<rpc_manager_space>(
                manager_space_conf{ get_num_segments() }
            )
        )
        , sharer_(
            mefdn::make_unique<sharer_space>(
                sharer_space_conf{ get_num_segments() }
            )
        )
        , protector_(
            protector_space_conf{ get_num_segments(), *sharer_, get_max_segment_size(), shm_obj_.get_fd() }
        )
        
        , manager_pr_(manager_->make_proxy_collective())
        , sharer_pr_(sharer_->make_proxy_collective())
        , protector_pr_(protector_.make_proxy_collective())
        
        , hist_(
            {
                protector_
            }
        )
        , fault_upgrader_(
            page_fault_upgrader::config{ protector_, hist_ }
        )
        , new_seg_id_offset_(0)
    {
        this->protector_.set_history(this->hist_);
        
        this->manager_->set_activater(this->sharer_pr_);
        
        this->sharer_->set_manager(this->manager_pr_);
        
        MEFDN_LOG_DEBUG("msg:Initialize DSM space.");
        
        // Barrier to initialize global variables.
        mecom::collective::barrier();
        
        #ifdef MEDSM_USE_GLOBAL_VAR
        init_global_var_seg();
        #endif
        
        // Barrier to initialize global variables.
        mecom::collective::barrier();
    }
    
    ~dsm_space()
    {
        MEFDN_LOG_DEBUG("msg:Finalize DSM space.");
        
        mecom::collective::barrier();
    }
    
private:
    // Note: Old GCC doesn't allow to use local class for template arguments.
    struct segment_conf {
        protector_space::proxy&     protector;
        segment_id_t                seg_id;
        mefdn::size_t              num_pages;
        mefdn::size_t              page_size;
        mefdn::size_t              block_size;
        void*                       app_ptr;
        void*                       sys_ptr;
        mefdn::size_t              index_in_file;
    };
    
public:
    virtual segment_ref make_segment(
        mefdn::size_t  size_in_bytes
    ,   mefdn::size_t  page_size_in_bytes
    ,   mefdn::size_t  block_size_in_bytes
    ) MEFDN_OVERRIDE
    {
        MEFDN_ASSERT(size_in_bytes <= get_max_segment_size());
        
        const auto seg_id = make_new_segment_id();
        
        return segment_ref(new dsm_segment(
            segment_conf{
                protector_pr_
            ,   seg_id
            ,   mefdn::roundup_divide(size_in_bytes, page_size_in_bytes)
            ,   page_size_in_bytes
            ,   block_size_in_bytes
            ,   get_segment_app_ptr(seg_id)
            ,   get_segment_sys_ptr(seg_id)
            ,   reinterpret_cast<mefdn::size_t>(get_segment_app_ptr(seg_id))
            }
        ));
    }
    
    virtual void read_barrier() MEFDN_OVERRIDE
    {
        this->hist_.read_barrier();
    }
    virtual void write_barrier() MEFDN_OVERRIDE
    {
        this->hist_.write_barrier();
    }
    virtual void async_read_barrier(mefdn::callback<void ()> cb) MEFDN_OVERRIDE
    {
        this->hist_.async_read_barrier(cb);
    }
    virtual void async_write_barrier(mefdn::callback<void ()> cb) MEFDN_OVERRIDE
    {
        this->hist_.async_write_barrier(cb);
    }
    
    virtual void pin(void* const ptr, const mefdn::size_t size_in_bytes) MEFDN_OVERRIDE
    {
        this->protector_.do_for_all_blocks_in(ptr, size_in_bytes, pin_callback{});
    }
    virtual void unpin(void* const ptr, const mefdn::size_t size_in_bytes) MEFDN_OVERRIDE
    {
        this->protector_.do_for_all_blocks_in(ptr, size_in_bytes, unpin_callback{});
    }
    
private:
    struct pin_callback
    {
        void operator() (protector_block_accessor& blk_ac)
        {
            // If the block is "invalid", upgrade to "clean".
            blk_ac.fetch();
            
            // If the block is "clean", upgrade to "dirty".
            blk_ac.touch();
            
            // Although pinned pages are fetched/touched here,
            // they are not marked as reconciled/flushed pages
            // because they cannot be until they are unpinned.
            
            // Pin the block.
            blk_ac.pin();
        }
    };
    struct unpin_callback
    {
        void operator() (protector_block_accessor& blk_ac)
        {
            // Unpin the block.
            blk_ac.unpin();
            
            // This block must be marked as a candidate
            // for both flushed and reconciled pages
            // because it is now in the "dirty" state.
        }
    };
    
public:
    virtual void enable_on_this_thread() MEFDN_OVERRIDE
    {
        this->fault_upgrader_.enable_on_this_thread();
    }
    virtual void disable_on_this_thread() MEFDN_OVERRIDE
    {
        this->fault_upgrader_.disable_on_this_thread();
    }
    
    std::string get_reg_name() noexcept
    {
        return fmt::format("medsm_cache_{}", mecom::current_process_id());
    }
    
    segment_id_t make_new_segment_id()
    {
        MEFDN_ASSERT(new_seg_id_offset_ < get_num_segments_per_proc());
        const segment_id_t seg_id =
            // TODO: magic number
            0x100000000000 / get_max_segment_size() +
            mecom::current_process_id() * get_num_segments_per_proc() + new_seg_id_offset_++;
        
        MEFDN_ASSERT(seg_id < get_num_segments());
        
        return seg_id;
    }
    mefdn::size_t get_num_segments_per_proc() noexcept {
        return 4; // TODO: magic number
    }
    
    void* get_segment_app_ptr(const segment_id_t seg_id) noexcept
    {
        return reinterpret_cast<void*>(seg_id * get_max_segment_size());
    }
    void* get_segment_sys_ptr(const segment_id_t seg_id) noexcept
    {
        return reinterpret_cast<void*>((get_num_segments() + seg_id) * get_max_segment_size());
    }
    
    mefdn::size_t get_num_segments() noexcept {
        return 1ull << 10; // TODO: magic number
    }
    mefdn::size_t get_max_segment_size() noexcept {
        return get_address_space_size() / get_num_segments(); // TODO: magic number
    }
    mefdn::size_t get_address_space_size() noexcept {
        return 0x400000000000;
        //return 0x300000000000;
    }
    
    #ifdef MEDSM_USE_GLOBAL_VAR
    void init_global_var_seg()
    {
        if (mecom::current_process_id() != 0) {
            // Only the process 0 creates the global variable segment.
            return;
        }
        
        const segment_id_t seg_id = 0;
        const mefdn::size_t seg_size = get_global_var_seg_size();
        
        if (seg_size == 0) {
            // There's no global variable.
            return;
        }
        
        const auto app_ptr = get_global_var_seg_start_ptr();
        const auto sys_ptr = get_segment_sys_ptr(seg_id);
        
        mefdn::unique_ptr<mefdn::uint8_t []> data(new mefdn::uint8_t[seg_size]);
        
        // Copy the initial data to a buffer (before mmap()).
        memcpy(data.get(), app_ptr, seg_size);
        
        const mefdn::size_t page_size_in_bytes = 4096; // TODO
        const mefdn::size_t block_size_in_bytes = 4096; // TODO
        
        global_var_seg_ =
            mefdn::make_unique<dsm_segment>(
                segment_conf{
                    protector_pr_
                ,   0 // segment_id
                ,   mefdn::roundup_divide(seg_size, page_size_in_bytes)
                ,   page_size_in_bytes
                ,   block_size_in_bytes
                ,   app_ptr
                ,   sys_ptr
                ,   reinterpret_cast<mefdn::size_t>(app_ptr)
                }
            );
        
        this->enable_on_this_thread();
        
        // Restore the initial data to the global buffer.
        const auto seg_ptr = global_var_seg_->get_ptr();
        MEFDN_ASSERT(seg_ptr == app_ptr);
        
        memcpy(seg_ptr, data.get(), seg_size);
        
        this->disable_on_this_thread();
    }
    
    void* get_global_var_seg_start_ptr() noexcept {
        return reinterpret_cast<void*>(&_dsm_data_begin);
    }
    mefdn::size_t get_global_var_seg_size() noexcept {
        return 
            reinterpret_cast<mefdn::int8_t*>(&_dsm_data_end) -
            reinterpret_cast<mefdn::int8_t*>(&_dsm_data_begin);
    }
    #endif
    
    shm_object                              shm_obj_;
    
    mefdn::unique_ptr<rpc_manager_space>   manager_;
    mefdn::unique_ptr<sharer_space>        sharer_;
    protector_space                         protector_;
    
    rpc_manager_space::proxy                manager_pr_;
    sharer_space::proxy                     sharer_pr_;
    protector_space::proxy                  protector_pr_;
    
    access_history                          hist_;
    page_fault_upgrader                     fault_upgrader_;
    
    segment_id_t                            new_seg_id_offset_;
    
    #ifdef MEDSM_USE_GLOBAL_VAR
    mefdn::unique_ptr<dsm_segment>         global_var_seg_;
    #endif
};

} // namespace medsm
} // namespace menps

