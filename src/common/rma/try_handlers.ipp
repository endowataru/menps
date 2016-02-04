
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
    typedef try_handlers            handlers_type;
    typedef CB                      cb_type;
    typedef mgbase::deferred<void>  result_type;
    typedef result_type (func_type)(cb_type&);
    
public:
    static result_type start(cb_type& cb) {
        cb.finished.store(false, mgbase_memory_order_seq_cst);
        
        if (Derived::try_(cb, mgbase::make_operation_store_release(&cb.finished, true))) {
            return test(cb);
        }
        else {
            mgcom::rma::poll(); // TODO
            
            return mgbase::make_deferred<mgbase::deferred<void> (cb_type&), &try_handlers::start>(cb);
        }
    }
    
private:
    static result_type test(cb_type& cb)
    {
        if (cb.finished.load(mgbase_memory_order_acquire)) {
            return mgbase::make_ready_deferred();
        }
        else {
            mgcom::rma::poll(); // TODO
            
            return mgbase::make_deferred<func_type, &handlers_type::test>(cb);
        }
    }
};

} // unnamed namespace

} // namespace detail

} // namespace untyped
} // namespace rma
} // namespace mgcom

