
#include <mgdev/ibv/device_context.hpp>
#include <mgdev/ibv/device_list.hpp>
#include <mgdev/ibv/protection_domain.hpp>
#include <mgdev/ibv/completion_queue.hpp>
#include <mgdev/ibv/queue_pair.hpp>
#include <mgbase/external/fmt.hpp>
#include <mgbase/string.hpp>
#include <iostream>

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
    
    const auto qp = ibv::make_queue_pair(pd.get(), &init_attr);
    
    print("LID: {}\n", my_id.lid);
    print("QP num: {}\n", qp.get_qp_num());
    
    ibv::global_qp_id other_qp_id{};
    other_qp_id.port_num = port;
    
    {
        print("Dest LID: ");
        
        std::string s;
        getline(std::cin, s);
        
        const auto lid = mgbase::stoul(s);
        other_qp_id.node_id =
            ibv::make_node_id_from_lid(
                static_cast<ibv::lid_t>(lid)
            );
    }
    {
        print("Dest QP num: ");
        
        std::string s;
        getline(std::cin, s);
        
        const auto qp_num = mgbase::stoul(s);
        other_qp_id.qp_num = static_cast<ibv::qp_num_t>(qp_num);
    }
    
    qp.connect_to(other_qp_id);
    
    return 0;
}

