
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mecom2/coll/mpi/mpi_coll.hpp>
#include <menps/mecom2/p2p/mpi/mpi_p2p.hpp>
#include <menps/medsm2/com/dsm_com_creator.hpp>

namespace menps {
namespace medsm2 {

class mpi_svm_space
{
    using com_itf_type = dsm_com_creator::dsm_com_itf_type;
    
    using proc_id_type = int;
    using size_type = mefdn::size_t;
    
public:
    explicit mpi_svm_space(com_itf_type& com);
    
    ~mpi_svm_space();
    
    void* coll_alloc_seg(size_type seg_size, size_type blk_size);
    
    void coll_alloc_global_var_seg(size_type seg_size, size_type blk_size, void* start_ptr);
    
    bool compare_exchange_strong_acquire(
        mefdn::uint32_t&    target // TODO: define atomic type
    ,   mefdn::uint32_t&    expected
    ,   mefdn::uint32_t     desired
    );
    
    void store_release(mefdn::uint32_t*, mefdn::uint32_t);
    
    //mefdn::uint64_t load_acquire(mefdn::uint64_t*);
    
    void barrier();
    
    void pin(void*, size_type);
    
    void unpin(void*, size_type);
    
    void enable_on_this_thread();
    
    void disable_on_this_thread();
    
    bool try_upgrade(void* ptr);
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace medsm2
} // namespace menps

