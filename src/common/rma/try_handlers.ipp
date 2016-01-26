
#pragma once

#include <mgcom/rma.hpp>
#include "common/rma/rma.hpp"

namespace mgcom {
namespace rma {
namespace untyped {

namespace detail {

namespace /*unnamed*/ {

template <typename Derived, typename CB>
class try_handlers
{
    typedef CB  cb_type;
    
public:
    static mgbase::deferred<void> start(cb_type& cb) {
        cb.finished = false;
        
        if (Derived::try_(cb, mgbase::make_operation_store_release(&cb.finished, true))) {
            return test(cb);
        }
        else {
            mgcom::rma::poll(); // TODO
            
            return mgbase::make_deferred<mgbase::deferred<void> (cb_type&), &try_handlers::start>(cb);
        }
    }
    
private:
    static mgbase::deferred<void> test(cb_type& cb)
    {
        if (mgbase::atomic_load(&cb.finished)) {
            return mgbase::make_ready_deferred();
        }
        else {
            mgcom::rma::poll(); // TODO
            
            return mgbase::make_deferred<mgbase::deferred<void> (cb_type&), &try_handlers::test>(cb);
        }
    }
};

} // unnamed namespace

} // namespace detail

} // namespace untyped
} // namespace rma
} // namespace mgcom

