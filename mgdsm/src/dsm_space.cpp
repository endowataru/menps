
#include "dsm_space.hpp"
#include <mgdsm/make_space.hpp>

namespace mgdsm {

namespace /*unnamed*/ {

struct initializer
{
    initializer() {
        rpc_manager_segment::proxy::register_handlers();
        sharer_space::proxy::register_handlers();
        sharer_block::accessor::register_handlers();
        protector_space::proxy::register_handlers();
    }
};

} // unnamed namespace

space_ref make_space()
{
    static initializer i;
    
    mgcom::collective::barrier();
    
    return space_ref(new dsm_space);
}

} // namespace mgdsm

