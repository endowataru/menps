
#include <mgult/sm/initializer.hpp>
//#include <mgult/sm/make_scheduler.hpp>
#include <mgult/sm/scheduler.hpp>
#include <mgult/scheduler_initializer.hpp>

namespace mgult {
namespace sm {

class initializer::impl
{
public:
    impl()
    {
        sched_ = mgbase::make_unique<sm_scheduler>();//make_scheduler();
        
        sm::set_scheduler(sched_);
        
        init_ = mgbase::make_unique<scheduler_initializer>(*sched_);
    }
    
    ~impl()
    {
        init_.reset();
        
        sm::set_scheduler(scheduler_ptr{});
        
        MGBASE_ASSERT(sched_.unique());
        
        sched_.reset();
    }
    
private:
    scheduler_ptr                               sched_;
    mgbase::unique_ptr<scheduler_initializer>   init_;
};

initializer::initializer()
    : impl_(new impl()) { }

initializer::~initializer() = default;

} // namespace sm
} // namespace mgult

