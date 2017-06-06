
#include "manager/rpc_manager_segment_proxy.hpp"
#include "sharer/sharer_space_proxy.hpp"
#include "page_fault_upgrader.hpp"
#include <mgdsm/space.hpp>
#include <string>
#include <mgbase/external/fmt.hpp>
#include <mgbase/arithmetic.hpp>
#include <mgbase/logger.hpp>
#include "access_history.hpp"
#include "protector/protector_space.hpp"
#include "protector/protector_space_proxy.hpp"
#include "dsm_segment.hpp"
#include "shm_object.hpp"

extern "C" {

extern void* _dsm_data_begin;
extern void* _dsm_data_end;

} // extern "C"

namespace mgdsm {

class dsm_space
    : public space
{
    struct manager_space_conf
    {
        mgbase::size_t      num_segments;
    };
    
    struct sharer_space_conf
    {
        mgbase::size_t      num_segments;
    };
    
    struct protector_space_conf
    {
        mgbase::size_t      num_segments;
        sharer_space&       sharer_sp;
        mgbase::size_t      max_seg_size;
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
            mgbase::make_unique<rpc_manager_space>(
                manager_space_conf{ get_num_segments() }
            )
        )
        , sharer_(
            mgbase::make_unique<sharer_space>(
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
        
        MGBASE_LOG_DEBUG("msg:Initialize DSM space.");
        
        // Barrier to initialize global variables.
        mgcom::collective::barrier();
        
        init_global_var_seg();
        
        // Barrier to initialize global variables.
        mgcom::collective::barrier();
    }
    
    ~dsm_space()
    {
        MGBASE_LOG_DEBUG("msg:Finalize DSM space.");
        
        mgcom::collective::barrier();
    }
    
private:
    // Note: Old GCC doesn't allow to use local class for template arguments.
    struct segment_conf {
        protector_space::proxy&     protector;
        segment_id_t                seg_id;
        mgbase::size_t              num_pages;
        mgbase::size_t              page_size;
        mgbase::size_t              block_size;
        void*                       app_ptr;
        void*                       sys_ptr;
        mgbase::size_t              index_in_file;
        bool                        copy_data;
    };
    
public:
    virtual segment_ref make_segment(
        mgbase::size_t  size_in_bytes
    ,   mgbase::size_t  page_size_in_bytes
    ,   mgbase::size_t  block_size_in_bytes
    ) MGBASE_OVERRIDE
    {
        MGBASE_ASSERT(size_in_bytes <= get_max_segment_size());
        
        const auto seg_id = make_new_segment_id();
        
        return segment_ref(new dsm_segment(
            segment_conf{
                protector_pr_
            ,   seg_id
            ,   mgbase::roundup_divide(size_in_bytes, page_size_in_bytes)
            ,   page_size_in_bytes
            ,   block_size_in_bytes
            ,   get_segment_app_ptr(seg_id)
            ,   get_segment_sys_ptr(seg_id)
            ,   reinterpret_cast<mgbase::size_t>(get_segment_app_ptr(seg_id))
            ,   false
            }
        ));
    }
    
    virtual void read_barrier() MGBASE_OVERRIDE
    {
        this->hist_.read_barrier();
    }
    virtual void write_barrier() MGBASE_OVERRIDE
    {
        this->hist_.write_barrier();
    }
    virtual void async_read_barrier(mgbase::callback<void ()> cb) MGBASE_OVERRIDE
    {
        this->hist_.async_read_barrier(cb);
    }
    virtual void async_write_barrier(mgbase::callback<void ()> cb) MGBASE_OVERRIDE
    {
        this->hist_.async_write_barrier(cb);
    }
    
    virtual void pin(void* const ptr, const mgbase::size_t size_in_bytes) MGBASE_OVERRIDE
    {
        this->protector_.do_for_all_blocks_in(ptr, size_in_bytes, pin_callback{});
    }
    virtual void unpin(void* const ptr, const mgbase::size_t size_in_bytes) MGBASE_OVERRIDE
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
    virtual void enable_on_this_thread() MGBASE_OVERRIDE
    {
        this->fault_upgrader_.enable_on_this_thread();
    }
    virtual void disable_on_this_thread() MGBASE_OVERRIDE
    {
        this->fault_upgrader_.disable_on_this_thread();
    }
    
    std::string get_reg_name() MGBASE_NOEXCEPT
    {
        return fmt::format("mgdsm_cache_{}", mgcom::current_process_id());
    }
    
    segment_id_t make_new_segment_id()
    {
        MGBASE_ASSERT(new_seg_id_offset_ < get_num_segments_per_proc());
        const segment_id_t seg_id =
            // TODO: magic number
            0x100000000000 / get_max_segment_size() +
            mgcom::current_process_id() * get_num_segments_per_proc() + new_seg_id_offset_++;
        
        MGBASE_ASSERT(seg_id < get_num_segments());
        
        return seg_id;
    }
    mgbase::size_t get_num_segments_per_proc() MGBASE_NOEXCEPT {
        return 4; // TODO: magic number
    }
    
    void* get_segment_app_ptr(const segment_id_t seg_id) MGBASE_NOEXCEPT
    {
        return reinterpret_cast<void*>(seg_id * get_max_segment_size());
    }
    void* get_segment_sys_ptr(const segment_id_t seg_id) MGBASE_NOEXCEPT
    {
        return reinterpret_cast<void*>((get_num_segments() + seg_id) * get_max_segment_size());
    }
    
    mgbase::size_t get_num_segments() MGBASE_NOEXCEPT {
        return 1ull << 10; // TODO: magic number
    }
    mgbase::size_t get_max_segment_size() MGBASE_NOEXCEPT {
        return get_address_space_size() / get_num_segments(); // TODO: magic number
    }
    mgbase::size_t get_address_space_size() MGBASE_NOEXCEPT {
        return 0x400000000000;
        //return 0x300000000000;
    }
    
    void init_global_var_seg()
    {
        if (mgcom::current_process_id() != 0) {
            // Only the process 0 creates the global variable segment.
            return;
        }
        
        const segment_id_t seg_id = 0;
        const mgbase::size_t seg_size = get_global_var_seg_size();
        
        if (seg_size == 0) {
            // There's no global variable.
            return;
        }
        
        const auto app_ptr = get_global_var_seg_start_ptr();
        const auto sys_ptr = get_segment_sys_ptr(seg_id);
        
        const mgbase::size_t page_size_in_bytes = 4096; // TODO
        const mgbase::size_t block_size_in_bytes = 4096; // TODO
        
        global_var_seg_ =
            mgbase::make_unique<dsm_segment>(
                segment_conf{
                    protector_pr_
                ,   0 // segment_id
                ,   mgbase::roundup_divide(seg_size, page_size_in_bytes)
                ,   page_size_in_bytes
                ,   block_size_in_bytes
                ,   app_ptr
                ,   sys_ptr
                ,   reinterpret_cast<mgbase::size_t>(app_ptr)
                ,   true
                }
            );
    }
    
    void* get_global_var_seg_start_ptr() MGBASE_NOEXCEPT {
        return reinterpret_cast<void*>(&_dsm_data_begin);
    }
    mgbase::size_t get_global_var_seg_size() MGBASE_NOEXCEPT {
        return 
            reinterpret_cast<mgbase::int8_t*>(&_dsm_data_end) -
            reinterpret_cast<mgbase::int8_t*>(&_dsm_data_begin);
    }
    
    shm_object                              shm_obj_;
    
    mgbase::unique_ptr<rpc_manager_space>   manager_;
    mgbase::unique_ptr<sharer_space>        sharer_;
    protector_space                         protector_;
    
    rpc_manager_space::proxy                manager_pr_;
    sharer_space::proxy                     sharer_pr_;
    protector_space::proxy                  protector_pr_;
    
    access_history                          hist_;
    page_fault_upgrader                     fault_upgrader_;
    
    segment_id_t                            new_seg_id_offset_;
    
    mgbase::unique_ptr<dsm_segment>         global_var_seg_;
};

} // namespace mgdsm

