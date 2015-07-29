
#include <mgcom.hpp>

#include <mgbase/stdint.hpp>

#include "impl.hpp"

#include <infiniband/verbs.h>
#include <mpi.h>
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
    static const mgbase::uint32_t num_cqe = 1;
    
public:
    void initialize(int* argc, char*** argv) {
        int provided;
        ::MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
        
        int size, rank;
        ::MPI_Comm_size(MPI_COMM_WORLD, &size);
        ::MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        
        current_process_id_ = static_cast<process_id_t>(rank);
        number_of_processes_ = static_cast<index_t>(size);
        
        local_qp_nums_  = new mgbase::uint32_t[number_of_processes_];
        remote_qp_nums_ = new mgbase::uint32_t[number_of_processes_];
        lids_           = new mgbase::uint16_t[number_of_processes_];
        
        init_ibv();
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
        
        query_port();
        exchange_qp_nums();
        exchange_lids();
        
        modify_qp();
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
        pd_ = ::ibv_alloc_pd(context_);
        if (pd_ == MGBASE_NULLPTR)
            throw ibv_error();
    }
    
    void create_cq() {
        cq_ = ::ibv_create_cq(context_, num_cqe, MGBASE_NULLPTR, MGBASE_NULLPTR, 0);
        if (cq_ == MGBASE_NULLPTR)
            throw ibv_error();
    }
    
    void create_qp() {
        ::ibv_qp_init_attr attr;
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
        
        for (process_id_t proc = 0; proc < number_of_processes_; ++proc) {
            ::ibv_qp* qp = ::ibv_create_qp(pd_, &attr);
            if (qp_[proc] == MGBASE_NULLPTR)
                throw ibv_error();
            
            ::ibv_qp_attr qp_attr;
            qp_attr.qp_state        = IBV_QPS_INIT;
            qp_attr.pkey_index      = 0;
            qp_attr.port_num        = 1; // 1 or 2
            qp_attr.qp_access_flags = IBV_ACCESS_LOCAL_WRITE;
            
            // Reset -> Init
            int ret = ::ibv_modify_qp(qp_[proc], &qp_attr,
                IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS);
            if (ret != 0)
                throw ibv_error();
            
            qp_[proc] = qp;
        }
    }
    
    void query_port() {
        // TODO: Port number is fixed to 0
        int ret = ibv_query_port(context_, 0, &port_attr_);
        if (ret != 0)
            throw ibv_error();
    }
    
    void exchange_qp_nums() {
        for (process_id_t proc = 0; proc < number_of_processes_; ++proc)
            local_qp_nums_[proc] = qp_[proc]->qp_num;
        
        int ret = MPI_Alltoall(
            local_qp_nums_, sizeof(local_qp_nums_[0]), MPI_BYTE,
            remote_qp_nums_, sizeof(remote_qp_nums_[0]), MPI_BYTE,
            MPI_COMM_WORLD
        );
        if (ret != MPI_SUCCESS)
            throw ibv_error();
    }
    
    void exchange_lids() {
        int ret = MPI_Allgather(
            &port_attr_.lid, sizeof(port_attr_.lid), MPI_BYTE,
            lids_, sizeof(lids_[0]), MPI_BYTE,
            MPI_COMM_WORLD
        );
        if (ret != MPI_SUCCESS)
            throw ibv_error();
    }
    
    void modify_qp() {
        for (index_t proc = 0; proc < number_of_processes_; proc++) {
            
        }
    }
    
    void modify_qp_init_to_rtr(::ibv_qp* qp, process_id_t proc) {
        ::ibv_qp_attr qp_attr;
        qp_attr.qp_state = IBV_QPS_RTR;
        qp_attr.path_mtu = IBV_MTU_4096;
        qp_attr.dest_qp_num = remote_qp_nums_[proc];
    }
    
    
    ::ibv_context*    context_;
    ::ibv_pd*         pd_;
    ::ibv_cq*         cq_;
    ::ibv_qp**        qp_;
    mgbase::uint32_t* local_qp_nums_;
    mgbase::uint32_t* remote_qp_nums_;
    ::ibv_port_attr   port_attr_;
    mgbase::uint16_t* lids_;
    
    process_id_t current_process_id_;
    index_t number_of_processes_;
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

