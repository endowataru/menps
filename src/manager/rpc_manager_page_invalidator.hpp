
#pragma once

#include "process_id_set.hpp"

namespace mgdsm {

class rpc_manager_page_invalidator
{
public:
    template <typename Conf>
    rpc_manager_page_invalidator(const Conf& conf)
        : readers_(conf.readers)
        , writers_(conf.writers)
    { }
    
    template <typename Seg>
    void send_to_all(const mgcom::process_id_t src_proc, Seg& seg, const page_id_t pg_id) const
    {
        for (const auto proc : this->readers_)
        {
            // A new writer must be different from the existing readers.
            //MGBASE_ASSERT(proc != src_proc);
            
            if (proc == src_proc)
            {
                // The existing reader is the same as the new writer.
                // The coherence information is sent via "acquire_write"'s RPC.
            }
            else
            {
                seg.enable_flush(proc, pg_id);
            }
        }
        for (const auto proc : this->writers_)
        {
            // A new writer must be different from the existing writers.
            MGBASE_ASSERT(proc != src_proc);
            
            seg.enable_diff(proc, pg_id);
        }
    }
    
private:
    process_id_set  readers_;
    process_id_set  writers_;
};

} // namespace mgdsm

