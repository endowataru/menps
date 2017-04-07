
#include <mgdev/ibv/device_context.hpp>
#include <mgdev/ibv/device_list.hpp>
#include <mgdev/ibv/protection_domain.hpp>
#include <mgdev/ibv/completion_queue.hpp>
#include <mgdev/ibv/queue_pair.hpp>
#include <mgbase/external/fmt.hpp>
#include <mgbase/string.hpp>
#include <mgbase/utility/move.hpp>
#include <iostream>
#include <mgbase/vector.hpp>

int main()
{
    namespace ibv = mgdev::ibv;
    
    using fmt::print;
    
    const ibv::port_num_t port = 1;
    
    const auto devs = ibv::get_device_list();
    const auto dev = ibv::open_device(devs[0]);
    
    const auto my_id = dev.get_node_id(port);
    //ibv::node_id_t other_id; // TODO
    
    const auto pd = ibv::make_protection_domain(dev.get());
    const auto cq = ibv::make_completion_queue(dev.get(), 1<<18);
    
    auto init_attr = ibv::make_default_rc_qp_init_attr();
    init_attr.send_cq = cq.get();
    init_attr.recv_cq = cq.get();
    
    mgbase::vector<ibv::queue_pair> qps;
    
    while (true)
    {
        auto qp = ibv::make_queue_pair(pd.get(), &init_attr);
        qps.push_back(mgbase::move(qp));
    }
    
    return 0;
}

