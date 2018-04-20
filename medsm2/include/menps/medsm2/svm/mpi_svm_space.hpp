
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>
#include <menps/mecom2/coll/mpi/mpi_coll.hpp>
#include <menps/mecom2/p2p/mpi/mpi_p2p.hpp>

namespace menps {
namespace medsm2 {

class mpi_svm_space
{
    using proc_id_type = int;
    using size_type = mefdn::size_t;
    
public:
    explicit mpi_svm_space(
        mecom2::mpi_rma&    rma
    ,   mecom2::mpi_coll&   coll
    ,   mecom2::mpi_p2p&    p2p
    );
    
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
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace medsm2
} // namespace menps

