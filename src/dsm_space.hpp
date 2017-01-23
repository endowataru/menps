
#include "app_space.hpp"
#include "manager/rpc_manager_segment_proxy.hpp"
#include "sharer/sharer_space_proxy.hpp"
#include "page_fault_upgrader.hpp"
#include "aliasing_mapped_region.hpp"
#include <mgdsm/space.hpp>
#include <string>
#include <mgbase/external/fmt.hpp>
#include "dsm_segment.hpp"
#include <mgbase/arithmetic.hpp>
#include <mgbase/logger.hpp>
#include "access_history.hpp"

namespace mgdsm {

class dsm_space
    : public space
    , public segment_locator
{
    struct manager_space_conf
    {
        mgbase::size_t      num_segments;
    };
    
    struct sharer_space_conf
    {
        mgbase::size_t      num_segments;
        segment_locator&    locator;
    };
    
public:
    dsm_space()
        : reg_name_(get_reg_name())
        , reg_(get_region_config())
        , manager_(
            mgbase::make_unique<rpc_manager_space>(
                manager_space_conf{ get_num_segments() }
            )
        )
        , manager_pr_(manager_->make_proxy_collective())
        , sharer_(
            mgbase::make_unique<sharer_space>(
                sharer_space_conf{ get_num_segments(), *this }
            )
        )
        , sharer_pr_(sharer_->make_proxy_collective())
        , app_prot_(
            {
                reg_.get_app_ptr()
            ,   get_max_segment_size()
            }
        )
        , app_sp_(app_prot_)
        , app_idx_(
            {
                *sharer_
            ,   reg_.get_app_ptr()
            ,   get_address_space_size()
            ,   get_max_segment_size()
            }
        )
        , hist_(
            {
                *sharer_
            ,   app_sp_
            }
        )
        , fault_upgrader_(
            page_fault_upgrader::config{ app_sp_, app_idx_, hist_ }
        )
        , new_seg_id_(100 * mgcom::current_process_id())
    {
        this->manager_->set_activater(this->sharer_pr_);
        
        this->sharer_->set_manager_proxy(this->manager_pr_);
        
        MGBASE_LOG_DEBUG("msg:Initialize DSM space.");
    }
    
    ~dsm_space()
    {
        MGBASE_LOG_DEBUG("msg:Finalize DSM space.");
        
        mgcom::collective::barrier();
    }
    
private:
    // Note: Old GCC doesn't allow to use local class for template arguments.
    struct segment_conf {
        rpc_manager_space::proxy&   manager;
        segment_id_t                seg_id;
        mgbase::size_t              num_pages;
        mgbase::size_t              page_size;
        mgbase::size_t              block_size;
        void*                       app_ptr;
    };
    
public:
    virtual segment_ref make_segment(
        mgbase::size_t  size_in_bytes
    ,   mgbase::size_t  page_size_in_bytes
    ,   mgbase::size_t  block_size_in_bytes
    ) MGBASE_OVERRIDE
    {
        const auto seg_id = make_new_segment_id();
        
        return segment_ref(new dsm_segment(
            segment_conf{
                manager_pr_
            ,   seg_id
            ,   mgbase::roundup_divide(size_in_bytes, page_size_in_bytes)
            ,   page_size_in_bytes
            ,   block_size_in_bytes
            ,   get_segment_app_ptr(seg_id)
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
    
    virtual void pin(void* const ptr, const mgbase::size_t size_in_bytes) MGBASE_OVERRIDE
    {
        this->app_idx_.do_for_all_blocks_in(ptr, size_in_bytes, pin_callback{*this});
    }
    virtual void unpin(void* const ptr, const mgbase::size_t size_in_bytes) MGBASE_OVERRIDE
    {
        // TODO: remove dynamic allocation
        std::vector<access_history::abs_block_id> ids;
        
        this->app_idx_.do_for_all_blocks_in(ptr, size_in_bytes, unpin_callback{*this, ids});
        
        MGBASE_RANGE_BASED_FOR(auto&& ablk_id, ids)
        {
            // Add this block as a flushed page.
            this->hist_.add_new_read(ablk_id);
            // Add this block as a reconciled page.
            this->hist_.add_new_write(ablk_id);
        }
    }
    
private:
    struct pin_callback
    {
        dsm_space& self;
        
        void operator() (sharer_block::accessor& blk_ac)
        {
            // If the block is "invalid", upgrade to "clean".
            self.app_sp_.fetch(blk_ac);
            
            // If the block is "clean", upgrade to "dirty".
            self.app_sp_.touch(blk_ac);
            
            // Although pinned pages are fetched/touched here,
            // they are not marked as reconciled/flushed pages
            // because they cannot be until they are unpinned.
            
            // Pin the block.
            self.app_sp_.pin(blk_ac);
        }
    };
    struct unpin_callback
    {
        dsm_space& self;
        std::vector<access_history::abs_block_id>& ids;
        
        void operator() (sharer_block::accessor& blk_ac)
        {
            const access_history::abs_block_id ablk_id{
                blk_ac.get_segment_id()
            ,   blk_ac.get_page_id()
            ,   blk_ac.get_block_id()
            };
            
            // Unpin the block.
            self.app_sp_.unpin(blk_ac);
            
            // This block must be marked as a candidate
            // for both flushed and reconciled pages
            // because it is now in the "dirty" state.
            
            ids.push_back(ablk_id);
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
    
    virtual void* get_segment_sys_ptr(const segment_id_t seg_id) MGBASE_NOEXCEPT MGBASE_OVERRIDE
    {
        const auto seg_size = get_max_segment_size();
        
        return mgbase::next_in_bytes(get_sys_ptr(), seg_id * seg_size);
    }
    
private:
    void* get_segment_app_ptr(const segment_id_t seg_id) MGBASE_NOEXCEPT
    {
        const auto seg_size = get_max_segment_size();
        
        return mgbase::next_in_bytes(get_app_ptr(), seg_id * seg_size);
    }
    
    aliasing_mapped_region::config get_region_config() {
        return {
            reg_name_.c_str()
        ,   get_address_space_size()
        ,   get_app_ptr() // Be careful for this order
        ,   get_sys_ptr()
        };
    }
    
    static std::string get_reg_name() MGBASE_NOEXCEPT
    {
        return fmt::format("mgdsm_cache_{}", mgcom::current_process_id());
    }
    
    void* get_sys_ptr() {
        return reinterpret_cast<void*>(0x30000000000);
    }
    void* get_app_ptr() {
        return mgbase::next_in_bytes(this->get_sys_ptr(), get_address_space_size());
    }
    
    segment_id_t make_new_segment_id() {
        return new_seg_id_++; // TODO
    }
    
    mgbase::size_t get_max_segment_size() MGBASE_NOEXCEPT {
        return get_address_space_size() / get_num_segments();
    }
    
    mgbase::size_t get_address_space_size() MGBASE_NOEXCEPT {
        return 1ull << 36;
    }
    
    mgbase::size_t get_num_segments() MGBASE_NOEXCEPT {
        return 1ull << 10;
    }
    
    const std::string                   reg_name_;
    aliasing_mapped_region              reg_;
    mgbase::unique_ptr<rpc_manager_space>   manager_;
    rpc_manager_space::proxy            manager_pr_;
    mgbase::unique_ptr<sharer_space>    sharer_;
    sharer_space::proxy                 sharer_pr_;
    app_space_protector                 app_prot_;
    app_space                           app_sp_;
    app_space_indexer                   app_idx_;
    access_history                      hist_;
    page_fault_upgrader                 fault_upgrader_;
    segment_id_t                        new_seg_id_;
};

} // namespace mgdsm

