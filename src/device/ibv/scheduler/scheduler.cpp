
#include "scheduler.hpp"
#include "serializer.hpp"
#include "device/ibv/rma/requester_base.hpp"
#include "device/ibv/command/atomic_buffer.hpp"
#include <mgbase/arithmetic.hpp>

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

class scheduled_rma_requester
    : public rma::ibv_requester_base<scheduled_rma_requester>
{
public:
    typedef ibv::command_code   command_code_type;
    
    scheduled_rma_requester(endpoint& ibv_ep, completion_selector& comp_sel, rma::allocator& alloc, mgcom::endpoint& ep)
    {
        const mgbase::size_t max_num_offload_threads = get_max_num_offload_threads();
        
        sers_.resize(max_num_offload_threads);
        
        mgbase::size_t qp_per_thread = mgbase::roundup_divide(ep.number_of_processes(), max_num_offload_threads);
        
        mgbase::size_t qp_from = 0;
        
        for (mgbase::size_t index = 0; index < max_num_offload_threads; ++index)
        {
            sers_[index] = new serializer(
                serializer::config{ ibv_ep, alloc, comp_sel, qp_from, qp_per_thread }
            );
            
            qp_from += qp_per_thread;
            
            if (qp_from >= ep.number_of_processes())
                break;
        }
        /*
        for (process_id_t proc = 0; proc < ep.number_of_processes(); ++proc)
        {
            sers_[proc] = new serializer(
                serializer::config{ ibv_ep, alloc, comp_sel, proc, 1 }
            );
        }*/
    }
    
    ~scheduled_rma_requester()
    {
        for (auto itr = sers_.begin(); itr != sers_.end(); ++itr)
            delete *itr;
    }
    
    scheduled_rma_requester(const scheduled_rma_requester&) = delete;
    scheduled_rma_requester& operator = (const scheduled_rma_requester&) = delete;
    
private:
    friend class rma::ibv_requester_base<scheduled_rma_requester>;
    
    template <typename Params, typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(
        const process_id_t      proc
    ,   const command_code_type code
    ,   Func&&                  func
    ) {
        return sers_[proc]->try_enqueue<Params>(proc, code, std::forward<Func>(func));
    }
    
    static mgbase::size_t get_max_num_offload_threads() MGBASE_NOEXCEPT
    {
        if (const char* const direct = std::getenv("MGCOM_IBV_MAX_NUM_OFFLOAD_THREADS"))
            return std::atoi(direct);
        else
            return 1; // Default
    }
    
    std::vector<serializer*> sers_;
};

} // unnamed namespace

mgbase::unique_ptr<rma::requester> make_scheduled_rma_requester(endpoint& ibv_ep, completion_selector& comp_sel, rma::allocator& alloc, mgcom::endpoint& ep)
{
    return mgbase::make_unique<scheduled_rma_requester>(ibv_ep, comp_sel, alloc, ep);
}

} // namespace ibv
} // namespace mgcom

