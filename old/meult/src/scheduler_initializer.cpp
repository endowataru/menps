
#include <menps/meult/scheduler_initializer.hpp>
#include <menps/meult/generic/basic_scheduler_initializer.hpp>
#include <menps/mectx/context_policy.hpp>
#include <menps/mefdn/utility.hpp>

namespace menps {
namespace meult {

namespace /*unnamed*/ {

class initializer_impl;

struct initializer_traits
{
    typedef initializer_impl    derived_type;
    
    using context_type = mectx::context<void*>;
    using transfer_type = mectx::transfer<void*>;
};

class initializer_impl
    : public basic_scheduler_initializer<initializer_traits>
    , public mectx::context_policy
{
public:
    explicit initializer_impl(root_scheduler& sched)
        : sched_(sched)
    { }
    
    static mefdn::size_t get_stack_size() noexcept
    {
        return 2048 * 1024; // TODO
    }
    
    template <typename F>
    void loop(F&& f)
    {
        sched_.loop(mefdn::forward<F>(f));
    }
    
private:
    root_scheduler& sched_;
};

} // unnamed namespace

class scheduler_initializer::impl
    : public initializer_impl
{
public:
    explicit impl(root_scheduler& sched)
        : initializer_impl(sched)
    {
        this->initialize();
    }
    
    ~impl() {
        this->finalize();
    }
};

scheduler_initializer::scheduler_initializer(root_scheduler& sched)
    : impl_(new impl(sched)) { }

scheduler_initializer::~scheduler_initializer() = default;

} // namespace meult
} // namespace menps

