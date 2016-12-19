
#include "app_space.hpp"
#include "manager/rpc_manager_segment_proxy.hpp"
#include "sharer/sharer_space.hpp"
#include "page_fault_upgrader.hpp"
#include "aliasing_mapped_region.hpp"
#include <mgdsm/space.hpp>
#include <string>
#include <mgbase/external/fmt.hpp>
#include "dsm_segment.hpp"
#include <mgbase/arithmetic.hpp>

namespace mgdsm {

class dsm_space
    : public space
    , public segment_locator
{
    struct sharer_space_conf
    {
        mgbase::size_t      num_segments;
        segment_locator&    locator;
    };
    
public:
    dsm_space()
        : reg_name_(get_reg_name())
        , reg_(get_region_config())
        , manager_(mgbase::make_unique<rpc_manager_space>())
        , manager_pr_(manager_->make_proxy_collective())
        , sharer_(
            mgbase::make_unique<sharer_space>(
                sharer_space_conf{ get_num_segments(), *this }
            )
        )
        , app_prot_(reg_.get_app_ptr())
        , app_sp_(app_prot_)
        , app_idx_(
            {
                *sharer_
            ,   reg_.get_app_ptr()
            ,   get_address_space_size()
            ,   get_address_space_size() / get_num_segments()
            }
        )
        , fault_upgrader_(
            page_fault_upgrader::config{ app_sp_, app_idx_ }
        )
        , new_seg_id_(100 * mgcom::current_process_id())
    {
        this->sharer_->set_manager_proxy(this->manager_pr_);
    }
    
    virtual segment_ref make_segment(
        mgbase::size_t  size_in_bytes
    ,   mgbase::size_t  page_size_in_bytes
    ) MGBASE_OVERRIDE
    {
        struct conf {
            rpc_manager_space::proxy& manager;
            segment_id_t        seg_id;
            mgbase::size_t num_pages;
            mgbase::size_t page_size;
        };
        
        return segment_ref(new dsm_segment(
            conf{
                manager_pr_
            ,   make_new_segment_id()
            ,   mgbase::roundup_divide(size_in_bytes, page_size_in_bytes)
            ,   page_size_in_bytes
            }
        ));
    }
    
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
        const auto seg_size = get_address_space_size() / get_num_segments();
        
        return mgbase::next_in_bytes(get_sys_ptr(), seg_id * seg_size);
    }
    
private:
    aliasing_mapped_region::config get_region_config() {
        return {
            reg_name_.c_str()
        ,   get_address_space_size()
        ,   get_sys_ptr()
        ,   get_app_ptr()
        };
    }
    
    static std::string get_reg_name() MGBASE_NOEXCEPT
    {
        return fmt::format("mgdsm_cache_{}", mgcom::current_process_id());
    }
    
    void* get_sys_ptr() {
        return reg_.get_sys_ptr();
    }
    void* get_app_ptr() {
        return reg_.get_app_ptr();
    }
    
    segment_id_t make_new_segment_id() {
        return new_seg_id_++; // TODO
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
    app_space_protector                 app_prot_;
    app_space                           app_sp_;
    app_space_indexer                   app_idx_;
    page_fault_upgrader                 fault_upgrader_;
    segment_id_t                        new_seg_id_;
};

} // namespace mgdsm

