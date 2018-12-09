
#pragma once

#include <menps/mefdn/crtp_base.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/assert.hpp>
#include <vector>

namespace menps {
namespace medsm {

template <typename Policy>
class basic_rpc_manager_segment_proxy
    : public Policy::interface_type
    , public mefdn::crtp_base<Policy>
{
    typedef typename Policy::derived_type   derived_type;
    typedef typename Policy::interface_type interface_type;
    
    typedef typename Policy::page_type      page_type;
    
    typedef typename Policy::space_type     space_type;
    
    typedef typename Policy::page_id_type   page_id_type;
    typedef typename Policy::segment_id_type segment_id_type;
    
    typedef typename interface_type::acquire_read_result    acquire_read_result_type;
    typedef typename interface_type::acquire_write_result   acquire_write_result_type;
    typedef typename interface_type::release_write_result   release_write_result_type;
    
    typedef typename Policy::handler_id_type    handler_id_type;
    
    typedef typename Policy::plptr_type             plptr_type;
    
    typedef typename Policy::invalidator_type   invalidator_type;
    
public:
    // local operations
    
    virtual mefdn::size_t get_page_size() const MEFDN_OVERRIDE
    {
        auto& self = this->derived();
        auto sg_ac = self.get_accessor();
        
        return sg_ac.get_page_size();
    }
    
    virtual mefdn::size_t get_num_pages() const MEFDN_OVERRIDE
    {
        auto& self = this->derived();
        auto sg_ac = self.get_accessor();
        
        return sg_ac.get_num_pages();
    }
    
    virtual mefdn::size_t get_block_size() const MEFDN_OVERRIDE
    {
        auto& self = this->derived();
        auto sg_ac = self.get_accessor();
        
        return sg_ac.get_block_size();
    }
    
public:
    // RPC operations
    
    static void register_handlers()
    {
        Policy::template register_handler<acquire_read_handler>();
        Policy::template register_handler<release_read_handler>();
        Policy::template register_handler<acquire_write_handler>();
        Policy::template register_handler<release_write_handler>();
        Policy::template register_handler<assign_reader_handler>();
        Policy::template register_handler<assign_writer_handler>();
    }
    
    MEFDN_NODISCARD
    virtual acquire_read_result_type acquire_read(const page_id_type pg_id) MEFDN_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(pg_id);
        const auto man_sp = sp.get_manager_space_at_proc(man_proc);
        
        const argument arg{ man_sp, seg_id, pg_id };
        
        const auto ret_msg =
            Policy::template call<acquire_read_handler>(
                man_proc
            ,   arg
            );
        
        return *ret_msg;
    }
    
    virtual void release_read(const page_id_type pg_id) MEFDN_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(pg_id);
        const auto man_sp = sp.get_manager_space_at_proc(man_proc);
        
        const argument arg{ man_sp, seg_id, pg_id };
        
        Policy::template call<release_read_handler>(
            man_proc
        ,   arg
        );
    }
    
    MEFDN_NODISCARD
    virtual acquire_write_result_type acquire_write(const page_id_type pg_id) MEFDN_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(pg_id);
        const auto man_sp = sp.get_manager_space_at_proc(man_proc);
        
        const argument arg{ man_sp, seg_id, pg_id };
        
        const auto ret_msg =
            Policy::template call<acquire_write_handler>(
                man_proc
            ,   arg
            );
        
        return *ret_msg;
    }
    
    MEFDN_NODISCARD
    virtual release_write_result_type release_write(const page_id_type pg_id) MEFDN_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(pg_id);
        const auto man_sp = sp.get_manager_space_at_proc(man_proc);
        
        const argument arg{ man_sp, seg_id, pg_id };
        
        const auto ret_msg =
            Policy::template call<release_write_handler>(
                man_proc
            ,   arg
            );
        
        return *ret_msg;
    }
    
    virtual void assign_reader(const page_id_type pg_id, const plptr_type& owner) MEFDN_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(pg_id);
        const auto man_sp = sp.get_manager_space_at_proc(man_proc);
        
        const assign_argument arg{ man_sp, seg_id, pg_id, owner };
        
        Policy::template call<assign_reader_handler>(
            man_proc
        ,   arg
        );
    }
    
    virtual void assign_writer(const page_id_type pg_id, const plptr_type& owner) MEFDN_OVERRIDE
    {
        auto& self = this->derived();
        auto& sp = self.get_space_proxy();
        
        const auto seg_id = self.get_segment_id();
        
        const auto man_proc = sp.get_manager_proc(pg_id);
        const auto man_sp = sp.get_manager_space_at_proc(man_proc);
        
        const assign_argument arg{ man_sp, seg_id, pg_id, owner };
        
        Policy::template call<assign_writer_handler>(
            man_proc
        ,   arg
        );
    }
    
private:
    struct argument {
        space_type*         sp;
        segment_id_type     seg_id;
        page_id_type        pg_id;
    };
    
    struct acquire_read_handler
    {
        static const handler_id_type handler_id = Policy::acquire_read_handler_id;
        
        typedef argument                    request_type;
        typedef acquire_read_result_type    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            const auto src_proc = sc.src_proc();
            
            // Look up the segment.
            auto seg_ac = sp.get_segment_accessor(seg_id);
            
            // Look up the page.
            auto pg_ac = seg_ac.get_page_accessor(pg_id);
            
            auto rep = sc.make_reply();
            
            // Call acquire_read() on the manager process.
            auto ret = pg_ac.acquire_read(src_proc);
            
            *rep = acquire_read_result_type{ ret.owner_plptr, ret.needs_flush };
            
            return rep;
        }
    };
    
    struct release_read_handler
    {
        static const handler_id_type handler_id = Policy::release_read_handler_id;
        
        typedef argument    request_type;
        typedef void        reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            const auto src_proc = sc.src_proc();
            
            // Look up the segment.
            auto seg_ac = sp.get_segment_accessor(seg_id);
            
            // Look up the page.
            auto pg_ac = seg_ac.get_page_accessor(pg_id);
            
            // Call release_read() on the manager process.
            pg_ac.release_read(src_proc);
            
            return sc.make_reply();
        }
    };
    
    struct acquire_write_handler
    {
        static const handler_id_type handler_id = Policy::acquire_write_handler_id;
        
        typedef argument                    request_type;
        typedef acquire_write_result_type   reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            const auto src_proc = sc.src_proc();
            
            auto rep = sc.make_reply();
            
            invalidator_type inv;
            
            {
                // Look up the segment.
                auto seg_ac = sp.get_segment_accessor(seg_id);
                
                // Look up the page.
                auto pg_ac = seg_ac.get_page_accessor(pg_id);
                
                // Call acquire_write() on the manager process.
                auto ret = pg_ac.acquire_write(src_proc);
                
                *rep = acquire_write_result_type{
                    ret.owner_plptr
                ,   ret.needs_flush
                ,   ret.needs_diff
                };
                
                inv = mefdn::move(ret.inv);
                
                // This is important to unlock these manager entries
                // in order to avoid deadlocking.
            }
            
            // Send invalidation messages to the sharer processes.
            inv.send_to_all(sp.get_activater(), src_proc, seg_id, pg_id);
            
            return rep;
        }
    };
    
    struct release_write_handler
    {
        static const handler_id_type handler_id = Policy::release_write_handler_id;
        
        typedef argument                    request_type;
        typedef release_write_result_type   reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            const auto src_proc = sc.src_proc();
            
            // Look up the segment.
            auto seg_ac = sp.get_segment_accessor(seg_id);
            
            // Look up the page.
            auto pg_ac = seg_ac.get_page_accessor(pg_id);
            
            auto ret = pg_ac.release_write(src_proc);
            
            auto rply = sc.make_reply();
            
            *rply = { ret.needs_flush };
            
            return rply;
        }
    };
    
    struct assign_argument {
        space_type*         sp;
        segment_id_type     seg_id;
        page_id_type        pg_id;
        plptr_type          owner;
    };
    
    struct assign_reader_handler
    {
        static const handler_id_type handler_id = Policy::assign_reader_handler_id;
        
        typedef assign_argument request_type;
        typedef void            reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            const auto& owner = rqst.owner;
            
            MEFDN_ASSERT(owner.proc == sc.src_proc());
            
            // Look up the segment.
            auto seg_ac = sp.get_segment_accessor(seg_id);
            
            // Look up the page.
            auto pg_ac = seg_ac.get_page_accessor(pg_id);
            
            // Call assign_reader() on the manager process.
            pg_ac.assign_reader(owner);
            
            return sc.make_reply();
        }
    };
    
    struct assign_writer_handler
    {
        static const handler_id_type handler_id = Policy::assign_writer_handler_id;
        
        typedef assign_argument request_type;
        typedef void            reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            const auto& owner = rqst.owner;
            
            MEFDN_ASSERT(owner.proc == sc.src_proc());
            
            // Look up the segment.
            auto seg_ac = sp.get_segment_accessor(seg_id);
            
            // Look up the page.
            auto pg_ac = seg_ac.get_page_accessor(pg_id);
            
            // Call assign_writer() on the manager process.
            pg_ac.assign_writer(owner);
            
            return sc.make_reply();
        }
    };
};

} // namespace medsm
} // namespace menps

