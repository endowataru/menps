
#pragma once

#include "basic_offload_history.hpp"
#include <mgdsm/ult.hpp>
#include <mgbase/callback.hpp>
#include "protector/protector_space.hpp"

namespace mgdsm {

class write_history;

struct write_history_policy
    : mgdsm::ult::ult_policy
    , dsm_base_policy
{
    typedef write_history               derived_type;
    typedef mgbase::callback<void ()>   callback_type;
};

class write_history
    : public basic_offload_history<write_history_policy>
{
public:
    struct config {
        protector_space&    sp;
    };
    
    explicit write_history(const config& conf)
        : conf_(conf)
    {
        this->start();
    }
    
    write_history(const write_history&) = delete;
    write_history& operator = (const write_history&) = delete;
    
    ~write_history()
    {
        this->stop();
    }
    
private:
    struct write_barrier_closure
    {
        void operator() (protector_block_accessor& blk)
        {
            // If reconcile is not needed, do nothing.
            blk.reconcile();
        }
    };
    
public: // XXX
    void downgrade(const abs_block_id id)
    {
        this->conf_.sp.do_for_block_at(id, write_barrier_closure{});
    }
    
private:
    const config conf_;
};

} // namespace mgdsm

