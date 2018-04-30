
#pragma once

#include <menps/mecom2/rma/basic_rma_itf.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_blocking_itf
    : public basic_rma_itf<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using base = basic_rma_itf<P>;
    
public:
    using typename base::proc_id_type;
    using typename base::size_type;
    
    using typename base::handle_type;
    
private:
    using typename base::ult_itf_type;
    using mutex_type = typename ult_itf_type::mutex;
    using mutex_unique_lock_type = typename ult_itf_type::unique_mutex_lock; // TODO
    
    using rma_sn_type = typename P::rma_sn_type;
    
public:
    template <typename RemotePtr, typename LocalPtr>
    void read(
        const proc_id_type  src_proc
    ,   RemotePtr&&         src_rptr
    ,   LocalPtr&&          dest_lptr
    ,   const size_type     num_elems
    ) {
        auto& self = this->derived();
        
        auto h = self.make_handle();
        
        h.read_nb(
            src_proc
        ,   mefdn::forward<RemotePtr>(src_rptr)
        ,   mefdn::forward<LocalPtr>(dest_lptr)
        ,   num_elems
        );
        
        this->flush(h);
    }
    
    template <typename RemotePtr, typename LocalPtr>
    void write(
        const proc_id_type  dest_proc
    ,   RemotePtr&&         dest_rptr
    ,   LocalPtr&&          src_lptr
    ,   const size_type     num_elems
    ) {
        auto& self = this->derived();
        
        auto h = self.make_handle();
        
        h.write_nb(
            dest_proc
        ,   mefdn::forward<RemotePtr>(dest_rptr)
        ,   mefdn::forward<LocalPtr>(src_lptr)
        ,   num_elems
        );
        
        this->flush(h);
    }
    
    template <typename TargetPtr, typename T>
    T compare_and_swap(
        const proc_id_type  target_proc
    ,   TargetPtr&&         target_rptr
    ,   const T             expected
    ,   const T             desired
    ) {
        auto& self = this->derived();
        
        T result{};
        {
            auto h = self.make_handle();
            
            h.compare_and_swap_nb(
                target_proc
            ,   mefdn::forward<TargetPtr>(target_rptr)
            ,   &expected
            ,   &desired
            ,   &result
            );
            
            this->flush(h);
        }
        
        return result;
    }
    
private:
    void flush(handle_type& h)
    {
        // Generate SN.
        rma_sn_type sn;
        {
            mutex_unique_lock_type lk(this->mtx_);
            sn = ++this->latest_sn_;
        }
        
        // Switch to a different thread.
        ult_itf_type::this_thread::yield();
        
        {
            mutex_unique_lock_type lk(this->mtx_);
            if (sn - this->oldest_sn_ >= 0) {
                MEFDN_LOG_VERBOSE(
                    "msg:Execute RMA flush.\t"
                    "sn:{}\t"
                    "oldest_sn:{}\t"
                    "latest_sn:{}"
                ,   sn
                ,   this->oldest_sn_
                ,   this->latest_sn_
                );
                
                // Execute an actual flush.
                h.flush();
                
                this->oldest_sn_ = this->latest_sn_;
            }
            else {
                MEFDN_LOG_VERBOSE(
                    "msg:Avoid calling RMA flush.\t"
                    "sn:{}\t"
                    "oldest_sn:{}\t"
                    "latest_sn:{}"
                ,   sn
                ,   this->oldest_sn_
                ,   this->latest_sn_
                );
            }
        }
    }
    
    mutex_type mtx_;
    rma_sn_type oldest_sn_ = 0;
    rma_sn_type latest_sn_ = 0;
};

} // namespace mecom2
} // namespace menps

