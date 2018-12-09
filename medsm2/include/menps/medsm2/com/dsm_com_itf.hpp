
#pragma once

#include <menps/medsm2/common.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class dsm_com_itf
{
public:
    using rma_itf_type = typename P::rma_itf_type;
    using coll_itf_type = typename P::coll_itf_type;
    using p2p_itf_type = typename P::p2p_itf_type;
    
    struct conf_t
    {
        rma_itf_type&   rma;
        coll_itf_type&  coll;
        p2p_itf_type&   p2p;
        p2p_itf_type&   p2p_lock;
    };
    
    template <typename Conf>
    explicit dsm_com_itf(const Conf& conf)
        : rma_(conf.rma)
        , coll_(conf.coll)
        , p2p_(conf.p2p)
        , p2p_lock_(conf.p2p_lock)
    {
        this->proc_id_ = get_coll().this_proc_id();
        this->num_procs_ = get_coll().get_num_procs();
    }
    
    using proc_id_type = typename coll_itf_type::proc_id_type;
    
    proc_id_type this_proc_id() { return proc_id_; }
    proc_id_type get_num_procs() { return num_procs_; }
    
    rma_itf_type& get_rma() { return rma_; }
    coll_itf_type& get_coll() { return coll_; }
    p2p_itf_type& get_p2p() { return this->p2p_; }
    p2p_itf_type& get_p2p_lock() { return this->p2p_; }
    
private:
    rma_itf_type& rma_;
    coll_itf_type& coll_;
    p2p_itf_type& p2p_;
    p2p_itf_type& p2p_lock_;
    proc_id_type proc_id_;
    proc_id_type num_procs_;
};

template <typename Rma, typename Coll, typename P2p>
struct dsm_com_policy
{
    using rma_itf_type = Rma;
    using coll_itf_type = Coll;
    using p2p_itf_type = P2p;
};

} // namespace medsm2
} // namespace menps

