
#include <mgcom.hpp>

#include <mgbase/stdint.hpp>

#include "impl.hpp"

#include <infiniband/verbs.h>
#include <errno.h>

namespace mgcom {

namespace {

struct ibv_error { };

class com_ibv
{
    static const mgbase::uint32_t max_send_wr = 128;
    static const mgbase::uint32_t max_recv_wr = 128;
    static const mgbase::uint32_t max_send_sge = 1; // Scatter/Gather
    static const mgbase::uint32_t max_recv_sge = 1;
    
public:
    void initialize(int* argc, char*** argv) {
        
    }
    
    bool try_write_async(
        local_address_t                local_address
    ,   remote_address_t               remote_address
    ,   index_t                        size_in_bytes
    ,   process_id_t                   dest_proc
    ,   notifier_t                     on_complete
    ) {
        
        request* req = alloc_request();
        
        ::ibv_sge sge;
        sge.addr   = get_absolute_address(local_address);
        sge.length = size_in_bytes;
        sge.lkey   = local_address.region.local_id;
        
        ::ibv_send_wr wr;
        wr.wr_id               = new_id();
        wr.next                = MGBASE_NULLPTR;
        wr.sg_list             = &sge;
        wr.num_sge             = 1;
        wr.opcode              = IBV_WR_RDMA_WRITE;
        wr.send_flags          = 0; // TODO
        wr.wr.rdma.remote_addr = get_absolute_address(remote_address);
        wr.wr.rdma.rkey        = remote_address.region.remote_id;
        
        ::ibv_send_wr* bad_wr;
        int err = ::ibv_post_send(qp_[dest_proc], &wr, &bad_wr);
        if (err == 0)
            return true;
        else if (err == ENOMEM)
            return false;
        else
            throw ibv_error();
    }
    
private:
    class request;
    
    uint64_t new_id();
    
    request* alloc_request();
    
    void init_ibv() {
        open_device();
        alloc_pd();
        create_qp();
        
    }
    
    void open_device() {
        int num_devices;
        ::ibv_device** dev_list = ::ibv_get_device_list(&num_devices);
        
        if (dev_list == MGBASE_NULLPTR || num_devices < 0)
            throw ibv_error();
        
        // Use dev_list[0] (TODO)
        
        context_ = ::ibv_open_device(dev_list[0]);
        if (context_ == MGBASE_NULLPTR)
            throw ibv_error();
        
        ::ibv_free_device_list(dev_list);
    }
    
    void alloc_pd() {
        
    }
    
    void create_qp() {
        
        ibv_qp_init_attr attr;
        attr.qp_context          = MGBASE_NULLPTR;
        attr.qp_type             = IBV_QPT_RC; // Reliable Connection (RC)
        attr.send_cq             = cq_;
        attr.recv_cq             = cq_;
        attr.srq                 = MGBASE_NULLPTR;
        attr.cap.max_send_wr     = max_send_wr;
        attr.cap.max_recv_wr     = max_recv_wr;
        attr.cap.max_send_sge    = max_send_sge;
        attr.cap.max_recv_sge    = max_recv_sge;
        attr.cap.max_inline_data = 1; // TODO
        attr.sq_sig_all          = 1;
        
        
        
        for (index_t proc = 0; proc < number_of_processes_; proc++) {
            qp_[proc] = ::ibv_create_qp(pd_, &attr);
        }
    }
    
    ::ibv_context* context_;
    ::ibv_pd*      pd_;
    ::ibv_cq*      cq_;
    ::ibv_qp**     qp_;
    mgbase::uint32_t number_of_processes_;
};

com_ibv g_com;

}

void initialize(int* argc, char*** argv) {
    g_com.initialize(argc, argv);
}


bool try_write_async(
    local_address_t                local_address
,   remote_address_t               remote_address
,   index_t                        size_in_bytes
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
)
{
    return g_com.try_write_async(local_address, remote_address, size_in_bytes, dest_proc, on_complete);
}

}

