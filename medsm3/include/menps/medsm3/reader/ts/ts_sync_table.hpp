
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class ts_sync_table
{
    using sig_buffer_type = typename P::sig_buffer_type;
    using sig_buf_set_type = typename P::sig_buf_set_type;

    using mtx_id_type = typename P::mtx_id_type;
    using mtx_table_type = typename P::mtx_table_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using id_allocator_type = typename P::id_allocator_type;

    using rel_sig_type = typename P::rel_sig_type;

public:
    explicit ts_sync_table(com_itf_type& com, const fdn::size_t num_locks)
        : com_(com)
    {
        struct mtx_table_conf {
            com_itf_type& com;
            fdn::size_t max_num_locks;
            fdn::size_t sig_size_in_bytes;
        };
        this->mtx_tbl_.coll_init(
            mtx_table_conf{ com, num_locks, rel_sig_type::get_max_size_in_bytes() }
        );
        this->mtx_id_alloc_.coll_make(com, num_locks);
    }

    sig_buf_set_type exchange_sig(sig_buffer_type sig)
    {
        auto& com = this->com_;
        auto& coll = com.get_coll();
        const auto this_proc = com.this_proc_id();
        const auto num_procs = com.get_num_procs();

        const auto sig_size = this->mtx_tbl_.get_sig_size();
        
        // Serialize the release signature to transfer via MPI.
        const auto this_buf = sig.serialize(sig_size);
        
        // Allocate an uninitialized buffer.
        const auto all_buf =
            fdn::make_unique_uninitialized<fdn::byte []>(sig_size * num_procs);
        
        // Collect the signatures from all of the processes.
        coll.allgather(this_buf.get(), all_buf.get(), sig_size);

        sig_buf_set_type sigs(num_procs);
        for (proc_id_type proc_id = 0; proc_id < num_procs; ++proc_id) {
            if (proc_id != this_proc) {
                // TODO: Remove deserialization.
                sigs[proc_id] = sig_buffer_type::deserialize_from(
                    &all_buf[sig_size * proc_id], sig_size);
            }
        }
        return sigs;
    }

    sig_buffer_type lock_mutex(const mtx_id_type mtx_id)
    {
        const auto lk_ret = this->mtx_tbl_.lock(this->com_, mtx_id);
        return { fdn::move(lk_ret.sig_buf) };
    }
    void unlock_mutex(const mtx_id_type mtx_id, const sig_buffer_type& sig)
    {
        this->mtx_tbl_.unlock(this->com_, mtx_id, sig);
    }

    mtx_id_type allocate_mutex() {
        return static_cast<mtx_id_type>(
            this->mtx_id_alloc_.allocate(this->com_)
        );
    }
    void deallocate_mutex(const mtx_id_type mtx_id) {
        this->mtx_id_alloc_.deallocate(this->com_, mtx_id);
    }

private:
    com_itf_type&       com_;
    mtx_table_type      mtx_tbl_;
    id_allocator_type   mtx_id_alloc_;
};

} // namespace medsm3
} // namespace menps

