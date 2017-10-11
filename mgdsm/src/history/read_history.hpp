
#pragma once

#include "basic_offload_history.hpp"
#include <mgdsm/ult.hpp>
#include <mgbase/callback.hpp>
#include "protector/protector_space.hpp"

namespace mgdsm {

class read_history;

struct read_history_policy
    : mgdsm::ult::ult_policy
    , dsm_base_policy
{
    typedef read_history               derived_type;
    typedef mgbase::callback<void ()>   callback_type;
};

class read_history
    : public basic_offload_history<read_history_policy>
{
public:
    struct config {
        protector_space&    sp;
    };
    
    explicit read_history(const config& conf)
        : conf_(conf)
    {
        this->start();
    }
    
    read_history(const read_history&) = delete;
    read_history& operator = (const read_history&) = delete;
    
    ~read_history()
    {
        this->stop();
    }
    
private:
    struct read_barrier_closure
    {
        void operator() (protector_block_accessor& blk)
        {
            // Reconcile the page first, then flush it.
            
            // If reconcile is not needed, do nothing.
            blk.reconcile();
            
            // If flush is not needed, do nothing.
            blk.flush();
        }
    };
    
public: // XXX
    void downgrade(const abs_block_id id)
    {
        this->conf_.sp.do_for_block_at(id, read_barrier_closure{});
    }
    
private:
    const config conf_;
};

} // namespace mgdsm
