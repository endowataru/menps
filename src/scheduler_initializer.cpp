
#include <mgult/scheduler_initializer.hpp>
#include <mgult/generic/basic_scheduler_initializer.hpp>
#include <mgult/generic/fcontext_worker_base.hpp>

namespace mgult {

namespace /*unnamed*/ {

class initializer_impl;

struct initializer_traits
    : fcontext_worker_traits_base
{
    typedef initializer_impl    derived_type;
};

class initializer_impl
    : public basic_scheduler_initializer<initializer_traits>
    , public fcontext_worker_base
{
public:
    explicit initializer_impl(root_scheduler& sched)
        : sched_(sched)
    { }
    
    static mgbase::size_t get_stack_size() MGBASE_NOEXCEPT
    {
        return 2048 * 1024; // TODO
    }
    
    template <typename F>
    void loop(F&& f)
    {
        sched_.loop(mgbase::forward<F>(f));
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

} // namespace mgult

