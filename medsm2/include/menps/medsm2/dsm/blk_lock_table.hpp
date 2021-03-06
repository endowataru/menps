
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
    
    using blk_unique_lock_type = typename P::blk_unique_lock_type;
    
    #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
    using sharer_map_type = typename P::sharer_map_type;
    #else
    using wr_ts_type = typename P::wr_ts_type;
    using rd_ts_type = typename P::rd_ts_type;
    #endif

    using size_type = typename P::size_type;
    
    size_type get_lad_size() {
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        return sizeof(proc_id_type) + this->ge_size_;
        #else
        return sizeof(global_entry_with_header);
        #endif
    }

public:
    template <typename Conf>
    void coll_make(const Conf& conf)
    {
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        this->ge_size_ = sharer_map_type::get_sharer_map_size(conf.com.get_num_procs());
        #endif
        base::coll_make(conf.com, conf.num_blks, this->get_lad_size());
    }
    
    struct lock_global_result {
        proc_id_type    owner;
        proc_id_type    last_writer_proc;
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        sharer_map_type sharers;
        #else
        wr_ts_type      home_wr_ts;
        rd_ts_type      home_rd_ts;
        #endif
    };
    
    lock_global_result lock_global(
        com_itf_type&               com
    ,   const blk_id_type           blk_id
    ,   const blk_pos_type          blk_pos
    ,   const blk_unique_lock_type& blk_lk
    ) {
        CMPTH_P_PROF_SCOPE(P, tx_lock_global);
        
        auto& self = this->derived();
        self.check_locked(blk_pos, blk_lk);
        
        auto& p2p = com.get_p2p();
        const auto tag = P::get_tag_from_blk_id(blk_id);
        
        auto& rma = com.get_rma();
        const auto ge_buf =
            rma.template make_unique_uninitialized<mefdn::byte []>(this->get_lad_size());
        const auto ge_buf_ptr = ge_buf.get();
        
        const auto lk_ret = base::lock_global(com, p2p, blk_pos, tag, ge_buf_ptr);
        
        const mefdn::byte* const ge_byte_ptr = ge_buf_ptr; // implicit conversion
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        const auto last_writer_proc = *reinterpret_cast<const proc_id_type*>(ge_byte_ptr);
        const auto ge_lad_rest_ptr = &ge_byte_ptr[sizeof(proc_id_type)];
        #else
        const auto geh = *reinterpret_cast<const global_entry_with_header*>(ge_byte_ptr);
        const auto last_writer_proc = geh.last_writer_proc;
        #endif
        
        return { lk_ret.owner, last_writer_proc,
            #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
            sharer_map_type::copy_from(ge_lad_rest_ptr, this->ge_size_)
            #else
            geh.ge.home_wr_ts, geh.ge.home_rd_ts
            #endif
            };
    }
    
    template <typename EndTransactionResult>
    void unlock_global(
        com_itf_type&                   com
    ,   const blk_id_type               blk_id
    ,   const blk_pos_type              blk_pos
    ,   const blk_unique_lock_type&     blk_lk
    ,   const lock_global_result&       glk_ret MEFDN_MAYBE_UNUSED
    ,   const EndTransactionResult&     et_ret
    ) {
        CMPTH_P_PROF_SCOPE(P, tx_unlock_global);
        
        auto& self = this->derived();
        self.check_locked(blk_pos, blk_lk);
        
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
        
        const auto last_writer_proc = et_ret.last_writer_proc;
        #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
        std::vector<fdn::byte> lad_buf(this->get_lad_size());
        std::memcpy(&lad_buf[0], &last_writer_proc, sizeof(proc_id_type));
        std::memcpy(&lad_buf[sizeof(proc_id_type)], et_ret.sharers.get_raw(), this->ge_size_);
        #else
        global_entry_with_header lad{ last_writer_proc, { et_ret.new_wr_ts, et_ret.new_rd_ts } };
        #endif
        base::unlock_global(com, p2p, blk_pos, tag,
            #ifdef MEDSM2_USE_DIRECTORY_COHERENCE
            lad_buf.data()
            #else
            &lad
            #endif
            );
    }
    
    #ifdef MEDSM2_ENABLE_MIGRATION
    bool check_owned(
        com_itf_type&               com
    ,   const blk_pos_type          blk_pos
    ,   const blk_unique_lock_type& blk_lk
    ) {
        auto& self = this->derived();
        self.check_locked(blk_pos, blk_lk);
        
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
    
    struct global_entry_with_header {
        proc_id_type    last_writer_proc;
        global_entry    ge;
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
        return reinterpret_cast<global_entry_with_header*>(p_raw)->ge;
    }
    #endif // MEDSM2_ENABLE_FAST_RELEASE
    #endif // MEDSM2_USE_DIRECTORY_COHERENCE
};

} // namespace medsm2
} // namespace menps

