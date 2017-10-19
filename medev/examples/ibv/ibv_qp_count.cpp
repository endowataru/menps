
#include <menps/medev/ibv/device_context.hpp>
#include <menps/medev/ibv/device_list.hpp>
#include <menps/medev/ibv/protection_domain.hpp>
#include <menps/medev/ibv/completion_queue.hpp>
#include <menps/medev/ibv/queue_pair.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <menps/mefdn/utility.hpp>
#include <string>
#include <iostream>

int main()
{
    namespace ibv = menps::medev::ibv;
    
    using fmt::print;
    
    const ibv::port_num_t port = 1;
    
    const auto devs = ibv::get_device_list();
    const auto dev = ibv::open_device(devs[0]);
    
    const auto my_id = dev.get_node_id(port);
    //ibv::node_id_t other_id; // TODO
    
    const auto pd = ibv::make_protection_domain(dev.get());
    const auto cq = ibv::make_completion_queue(dev.get());
    
    auto init_attr = ibv::make_default_rc_qp_init_attr();
    init_attr.send_cq = cq.get();
    init_attr.recv_cq = cq.get();
    
    menps::mefdn::size_t num_qps = 0;
    
    try {
        while (true)
        {
            auto qp = ibv::make_queue_pair(pd.get(), &init_attr);
            const auto qp_num = qp.get_qp_num();
            
            print("QP Num = {}\n", qp_num);
            
            // Explicitly leak the resource.
            qp.release();
            
            ++num_qps;
        }
    } catch (...) {
        // Ignore the exception.
    }
    
    print("Number of QPs : {}\n", num_qps);
    
    return 0;
}

