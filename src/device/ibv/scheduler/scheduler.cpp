
#include "scheduler.hpp"
#include "serializer.hpp"
#include "device/ibv/rma/requester_base.hpp"
#include "device/ibv/command/atomic_buffer.hpp"

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

class scheduled_rma_requester
    : public rma::ibv_requester_base<scheduled_rma_requester>
{
public:
    typedef ibv::command_code   command_code_type;
    
    scheduled_rma_requester(endpoint& ibv_ep, completer& comp, rma::allocator& alloc, mgcom::endpoint& ep)
        #if 0
        : ep_(ibv_ep)
        , comp_(comp)
        , atomic_buf_(alloc)
        #endif
        : sers_(ep.number_of_processes())
    {
        for (process_id_t proc = 0; proc < ep.number_of_processes(); ++proc)
        {
            sers_[proc] = new serializer(
                serializer::config{ ibv_ep, alloc, comp, proc, 1 }
            );
        }
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
    
    #if 0
    completer& get_completer() const MGBASE_NOEXCEPT {
        return comp_;
    }
    atomic_buffer& get_atomic_buffer() MGBASE_NOEXCEPT {
        return atomic_buf_;
    }
    #endif
    
    template <typename Params, typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(
        const process_id_t      proc
    ,   const command_code_type code
    ,   Func&&                  func
    ) {
        return sers_[proc]->try_enqueue<Params>(proc, code, std::forward<Func>(func));
    }
    
private:
    #if 0
    ibv::endpoint& ep_;
    ibv::completer& comp_;
    ibv::atomic_buffer atomic_buf_;
    #endif
    std::vector<serializer*> sers_;
};

} // unnamed namespace

mgbase::unique_ptr<rma::requester> make_scheduled_rma_requester(endpoint& ibv_ep, completer& comp, rma::allocator& alloc, mgcom::endpoint& ep)
{
    return mgbase::make_unique<scheduled_rma_requester>(ibv_ep, comp, alloc, ep);
}

} // namespace ibv
} // namespace mgcom

