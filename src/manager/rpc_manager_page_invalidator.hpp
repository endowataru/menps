
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
    void send_to_all(Seg& seg, const page_id_t pg_id) const
    {
        // FIXME
        #if 0
        for (const auto proc : this->readers_) {
            seg.enable_flush(proc, pg_id);
        }
        for (const auto proc : this->writers_) {
            seg.enable_diff(proc, pg_id);
        }
        #endif
    }
    
private:
    process_id_set  readers_;
    process_id_set  writers_;
};

} // namespace mgdsm

