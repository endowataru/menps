
#pragma once

#include <mgbase/crtp_base.hpp>

namespace mgdsm {

template <typename Policy>
class basic_sharer_space_proxy
    : public Policy::interface_type
    , public mgbase::crtp_base<Policy>
{
    typedef typename Policy::derived_type       derived_type;
    
    typedef typename Policy::space_type         space_type;
    
    typedef typename Policy::segment_id_type    segment_id_type;
    typedef typename Policy::page_id_type       page_id_type;
    
    typedef typename Policy::handler_id_type    handler_id_type;
    typedef typename Policy::process_id_type    process_id_type;
    
public:
    static void register_handlers()
    {
        Policy::template register_handler<enable_flush_handler>();
        Policy::template register_handler<enable_diff_handler>();
    }
    
    virtual void enable_flush(
        const process_id_type   proc
    ,   const segment_id_type   seg_id
    ,   const page_id_type      pg_id
    ) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        
        const request req{
            self.get_sharer_space_at_proc(proc)
        ,   seg_id
        ,   pg_id
        };
        
        Policy::template call<enable_flush_handler>(
            proc
        ,   req
        );
    }
    
    virtual void enable_diff(
        const process_id_type   proc
    ,   const segment_id_type   seg_id
    ,   const page_id_type      pg_id
    ) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        
        const request req{
            self.get_sharer_space_at_proc(proc)
        ,   seg_id
        ,   pg_id
        };
        
        Policy::template call<enable_diff_handler>(
            proc
        ,   req
        );
    }
    
private:
    struct request {
        space_type*     sp;
        segment_id_type seg_id;
        page_id_type    pg_id;
    };
    
    struct enable_flush_handler
    {
        static const handler_id_type handler_id = Policy::enable_flush_handler_id;
        
        typedef request request_type;
        typedef void    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            auto seg_ac = sp.get_segment_accessor(seg_id);
            
            // Directly access the page entry to avoid deadlocking.
            seg_ac.enable_flush(pg_id);
            
            return sc.make_reply();
        }
    };
    
    struct enable_diff_handler
    {
        static const handler_id_type handler_id = Policy::enable_diff_handler_id;
        
        typedef request request_type;
        typedef void    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto pg_id = rqst.pg_id;
            
            auto seg_ac = sp.get_segment_accessor(seg_id);
            
            // Directly access the page entry to avoid deadlocking.
            seg_ac.enable_diff(pg_id);
            
            return sc.make_reply();
        }
    };
};

} // namespace mgdsm

