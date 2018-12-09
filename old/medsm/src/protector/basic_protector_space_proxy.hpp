
#pragma once

#include <menps/mefdn/crtp_base.hpp>

namespace menps {
namespace medsm {

template <typename Policy>
class basic_protector_space_proxy
    : public mefdn::crtp_base<Policy>
{
    typedef typename Policy::derived_type       derived_type;
    
    typedef typename Policy::space_type         space_type;
    typedef typename Policy::segment_id_type    segment_id_type;
    
    typedef typename Policy::process_id_type    process_id_type;
    typedef typename Policy::handler_id_type    handler_id_type;
    
public:
    
    typedef typename Policy::create_conf_type   create_conf_type;
    
    static void register_handlers()
    {
        Policy::template register_handler<create_segment_handler>();
    }
    
    void create_segment(const segment_id_type seg_id, const create_conf_type& conf)
    {
        auto& self = this->derived();
        
        for (process_id_type proc = 0; proc < Policy::number_of_processes(); ++proc)
        {
            auto* man_sp = self.get_protector_space_at_proc(proc);
            
            typename create_segment_handler::request_type req{
                man_sp
            ,   seg_id
            ,   conf
            };
            
            Policy::template call<create_segment_handler>(
                proc
            ,   req
            );
        }
    }
    
private:
    struct create_segment_handler
    {
        static const handler_id_type handler_id = Policy::create_segment_handler_id;
        
        struct request_type {
            space_type*         sp;
            segment_id_type     seg_id;
            create_conf_type    conf;
        };
        typedef void    reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            auto& sp = *rqst.sp;
            const auto seg_id = rqst.seg_id;
            const auto& conf = rqst.conf;
            
            sp.make_segment(seg_id, conf);
            
            return sc.make_reply();
        }
    };
};

} // namespace medsm
} // namespace menps

