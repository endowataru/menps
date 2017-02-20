
#pragma once

#include "space_coherence_activater.hpp"
#include "process_id_set.hpp"

namespace mgdsm {

class rpc_manager_page_invalidator
{
public:
    rpc_manager_page_invalidator() = default;
    
    template <typename Conf>
    rpc_manager_page_invalidator(const Conf& conf)
        : readers_(conf.readers)
        , writers_(conf.writers)
    { }
    
    void send_to_all(
        space_coherence_activater&  actv
    ,   const mgcom::process_id_t   src_proc
    ,   const segment_id_t          seg_id
    ,   const page_id_t             pg_id
    ) const
    {
        MGBASE_RANGE_BASED_FOR(const auto proc, this->readers_)
        {
            if (proc == src_proc)
            {
                // The existing reader is the same as the new writer.
                // The coherence information is sent via "acquire_write"'s RPC reply.
            }
            else
            {
                actv.enable_flush(proc, seg_id, pg_id);
            }
        }
        MGBASE_RANGE_BASED_FOR(const auto proc, this->writers_)
        {
            // A new writer must be different from the existing writers.
            MGBASE_ASSERT(proc != src_proc);
            
            actv.enable_diff(proc, seg_id, pg_id);
        }
    }
    
private:
    process_id_set  readers_;
    process_id_set  writers_;
};

} // namespace mgdsm

