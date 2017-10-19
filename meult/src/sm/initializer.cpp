
#include <menps/meult/sm/initializer.hpp>
//#include <menps/meult/sm/make_scheduler.hpp>
#include <menps/meult/sm/scheduler.hpp>
#include <menps/meult/scheduler_initializer.hpp>

namespace menps {
namespace meult {
namespace sm {

class initializer::impl
{
public:
    impl()
    {
        sched_ = mefdn::make_unique<sm_scheduler>();//make_scheduler();
        
        sm::set_scheduler(sched_);
        
        init_ = mefdn::make_unique<scheduler_initializer>(*sched_);
    }
    
    ~impl()
    {
        init_.reset();
        
        sm::set_scheduler(scheduler_ptr{});
        
        MEFDN_ASSERT(sched_.unique());
        
        sched_.reset();
    }
    
private:
    scheduler_ptr                               sched_;
    mefdn::unique_ptr<scheduler_initializer>   init_;
};

initializer::initializer()
    : impl_(new impl()) { }

initializer::~initializer() = default;

} // namespace sm
} // namespace meult
} // namespace menps

