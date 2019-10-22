
#pragma once

#include <menps/medsm2/dsm/queue_lock_table.hpp>
#include <menps/medsm2/dsm/fixed_lock_table.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class blk_lock_table;

template <typename P>
struct blk_lock_table_policy
{
    using com_itf_type = typename P::com_itf_type;
    using atomic_int_type = typename P::atomic_int_type;
    
    // A lock index corresponds to a block position.
    using lock_pos_type = typename P::blk_pos_type;
    
    using size_type = typename P::size_type;
    using p2p_tag_type = typename P::p2p_tag_type;
    
    template <typename Elem>
    using alltoall_buffer = typename P::template alltoall_buffer<Elem>;

    using prof_aspect_type = typename P::prof_aspect_type;
};

template <typename P>
class blk_lock_table
    #ifdef MEDSM2_ENABLE_MIGRATION
    : private queue_lock_table<blk_lock_table_policy<P>>
    #else
    : private fixed_lock_table<blk_lock_table_policy<P>>
    #endif
{
    MEFDN_DEFINE_DERIVED(P)
    
    #ifdef MEDSM2_ENABLE_MIGRATION
    using base = queue_lock_table<blk_lock_table_policy<P>>;
    #else
    using base = fixed_lock_table<blk_lock_table_policy<P>>;
    #endif
    
    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    
    using blk_id_type = typename P::blk_id_type;
    using blk_pos_type = typename P::blk_pos_type;
    using unique_lock_type = typename P::unique_lock_type;
    
    #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
    using sharer_map_type = typename P::sharer_map_type;
    #else
    using wr_ts_type = typename P::wr_ts_type;
    using rd_ts_type = typename P::rd_ts_type;
    #endif

    using size_type = typename P::size_type;
    
public:
    template <typename Conf>
    void coll_make(const Conf& conf)
    {
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        this->ge_size_ = sharer_map_type::get_sharer_map_size(conf.com.get_num_procs());
        #endif
        base::coll_make(conf.com, conf.num_blks,
            #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
            this->ge_size_
            #else
            sizeof(global_entry)
            #endif
            );
    }
    
    struct lock_global_result {
        proc_id_type    owner;
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        sharer_map_type sharers;
        #else
        wr_ts_type      home_wr_ts;
        rd_ts_type      home_rd_ts;
        #endif
    };
    
    lock_global_result lock_global(
        com_itf_type&           com
    ,   const blk_id_type       blk_id
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        CMPTH_P_PROF_SCOPE(P, tx_lock_global);
        
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        auto& p2p = com.get_p2p();
        const auto tag = P::get_tag_from_blk_id(blk_id);
        
        auto& rma = com.get_rma();
        const auto ge_buf =
            rma.template make_unique_uninitialized<mefdn::byte []>(
                #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
                this->ge_size_
                #else
                sizeof(global_entry)
                #endif
                );
        const auto ge_buf_ptr = ge_buf.get();
        
        const auto lk_ret = base::lock_global(com, p2p, blk_pos, tag, ge_buf_ptr);
        
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        const mefdn::byte* const ge_byte_ptr = ge_buf_ptr; // implicit conversion
        const auto ge = *reinterpret_cast<const global_entry*>(ge_byte_ptr); // TODO: remove cast
        #endif
        
        return { lk_ret.owner,
            #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
            sharer_map_type::copy_from(ge_buf_ptr, this->ge_size_)
            #else
            ge.home_wr_ts, ge.home_rd_ts
            #endif
            };
    }
    
    template <typename EndTransactionResult>
    void unlock_global(
        com_itf_type&               com
    ,   const blk_id_type           blk_id
    ,   const blk_pos_type          blk_pos
    ,   const unique_lock_type&     lk
    ,   const lock_global_result&   glk_ret MEFDN_MAYBE_UNUSED
    ,   const EndTransactionResult& et_ret
    ) {
        CMPTH_P_PROF_SCOPE(P, tx_unlock_global);
        
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const auto new_owner MEFDN_MAYBE_UNUSED =
            #ifdef MEDSM2_ENABLE_MIGRATION
            com.this_proc_id();
            #else
            glk_ret.owner;
            #endif
        MEFDN_ASSERT(et_ret.new_owner == new_owner);
        
        #ifdef MEDSM2_ENABLE_LAZY_MERGE
        auto& rma = com.get_rma();
        
        // Complete writing on the previous owner.
        rma.flush(glk_ret.owner);
        #endif
        
        auto& p2p = com.get_p2p();
        const auto tag = P::get_tag_from_blk_id(blk_id);
        
        #ifndef MEDSM2_USE_DIRECTORY_COHERENCE
        global_entry ge{ et_ret.new_wr_ts, et_ret.new_rd_ts };
        #endif
        base::unlock_global(com, p2p, blk_pos, tag,
            #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
            et_ret.sharers.get_raw()
            #else
            &ge
            #endif
            );
    }
    
    #ifdef MEDSM2_ENABLE_MIGRATION
    bool check_owned(
        com_itf_type&           com
    ,   const blk_pos_type      blk_pos
    ,   const unique_lock_type& lk
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, lk);
        
        const bool ret = base::check_owned(com, blk_pos);
        if (!ret) {
            CMPTH_P_PROF_ADD(P, tx_migrate);
        }
        return ret;
    }
    #endif
    
    #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
private:
    size_type ge_size_ = 0;

    #else
private:
    struct global_entry {
        // The timestamp that can be read by other writers.
        // Only modified by the local process.
        wr_ts_type  home_wr_ts;
        // The timestamp when this block must be self-invalidated.
        // If this process is the owner of this block,
        // this member may be modified by other reader processes.
        rd_ts_type  home_rd_ts;
    };
    
    #ifdef MEDSM2_ENABLE_FAST_RELEASE
public:
    global_entry read_lock_entry(const blk_pos_type blk_pos) {
        // TODO: atomicity
        const auto ge = this->get_lock_entry(blk_pos);
        return ge;
    }
    void write_lock_entry(
        const blk_pos_type  blk_pos
    ,   const wr_ts_type    home_wr_ts
    ,   const rd_ts_type    home_rd_ts
    ) {
        auto& ge = this->get_lock_entry(blk_pos);
        // TODO: atomicity
        ge = global_entry{home_wr_ts, home_rd_ts};
    }
    
private:
    global_entry& get_lock_entry(const blk_pos_type blk_pos) {
        const auto p = base::get_local_lad_at(blk_pos);
        mefdn::byte* const p_raw = p; // implicit conversion
        return *reinterpret_cast<global_entry*>(p_raw);
    }
    #endif // MEDSM2_ENABLE_FAST_RELEASE
    #endif // MEDSM2_USE_DIRECTORY_COHERENCE
};

} // namespace medsm2
} // namespace menps

