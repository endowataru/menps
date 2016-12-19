
#include "basic_rpc_manager_segment_shard.hpp"
#include "rpc_manager_segment_accessor.hpp"
#include "rpc_manager_page_accessor.hpp"
#include "rpc_manager_segment_proxy.hpp"

/*#include "basic_rpc_manager_space_proxy.hpp"
#include "rpc_manager_page_accessor.hpp"
#include "rpc_manager_space_proxy.hpp"*/

namespace mgdsm {

void f(rpc_manager_page::accessor& a) {
    a.acquire_read(0);
    a.release_read(0);
    a.acquire_write(0);
    a.release_write(0);
}

void g(rpc_manager_segment::proxy& pr) {
    rpc_manager_segment::proxy::register_handlers();
    pr.acquire_read(0);
    pr.release_read(0);
    pr.acquire_write(0);
    pr.release_write(0);
}

}


#if 0

#include "rpc_manager_segment_proxy.hpp"
#include "rpc_manager_space.hpp"

namespace mgdsm {

void f(rpc_manager_segment::proxy& p)
{
    //rpc_manager_segment::proxy p;
    p.acquire_read_page(0);
    p.release_read_page(0);
    p.acquire_write_page(0);
    p.release_write_page(0);
}

void g()
{
    struct conf {
        mgbase::size_t num_segments;
    };
    
    rpc_manager_space sp(conf{0});
}

} // namespace mgdsm

#endif

