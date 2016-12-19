
#include "dsm_space.hpp"
#include <mgdsm/space_ref.hpp>

namespace mgdsm {

space_ref make_space()
{
    return space_ref(new dsm_space);
}

} // namespace mgdsm

