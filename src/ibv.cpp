
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
    
    class connection {
    public:
        mgbase::uint32_t get_qp_num() {
            return qp_->qp_num;
        }
        
        void create(::ibv_cq* cq, ::ibv_pd* pd) {
            ::ibv_qp_init_attr attr = ::ibv_qp_init_attr();
            attr.qp_context          = MGBASE_NULLPTR;
            attr.qp_type             = IBV_QPT_RC; // Reliable Connection (RC)
            attr.send_cq             = cq;
            attr.recv_cq             = cq;
            attr.srq                 = MGBASE_NULLPTR;
            attr.cap.max_send_wr     = max_send_wr;
            attr.cap.max_recv_wr     = max_recv_wr;
            attr.cap.max_send_sge    = max_send_sge;
            attr.cap.max_recv_sge    = max_recv_sge;
            attr.cap.max_inline_data = 1; // TODO
            attr.sq_sig_all          = 1;
            
            qp_ = ::ibv_create_qp(pd, &attr);
            if (qp_ == MGBASE_NULLPTR)
                throw ibv_error();
        }
        
        void start(mgbase::uint32_t qp_num, mgbase::uint16_t lid, const ::ibv_device_attr& device_attr) {
            modify_qp_reset_to_init();
            modify_qp_init_to_rtr(qp_num, lid, device_attr);
            modify_qp_rtr_to_rts();
        }
        
    private:
        void modify_qp_reset_to_init() {
            ::ibv_qp_attr qp_attr = ::ibv_qp_attr();
            qp_attr.qp_state        = IBV_QPS_INIT;
            qp_attr.pkey_index      = 0;
            qp_attr.port_num        = 1; // 1 or 2
            qp_attr.qp_access_flags = IBV_ACCESS_LOCAL_WRITE;
            
            // Reset -> Init
            int ret = ::ibv_modify_qp(qp_, &qp_attr,
                IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS);
            if (ret != 0)
                throw ibv_error();
        }
        
        void modify_qp_init_to_rtr(mgbase::uint32_t qp_num, mgbase::uint16_t lid, const ::ibv_device_attr& device_attr) {
            ::ibv_qp_attr attr = ::ibv_qp_attr();
            attr.qp_state              = IBV_QPS_RTR;
            attr.path_mtu              = IBV_MTU_4096;
            attr.dest_qp_num           = qp_num;
            attr.rq_psn                = 0; // PSN starts from 0
            attr.max_dest_rd_atomic    = device_attr.max_qp_rd_atom;
            attr.max_rd_atomic         = 0;
            attr.min_rnr_timer         = 0; // Arbitary from 0 to 31 (TODO: Is it true?)
            attr.ah_attr.is_global     = 0; // Doesn't use Global Routing Header (GRH)
            attr.ah_attr.dlid          = lid;
            attr.ah_attr.sl            = 0;
            attr.ah_attr.src_path_bits = 0;
            attr.ah_attr.port_num      = 1; // 1 or 2
            attr.ah_attr.static_rate   = 0;
            
            // Init -> RTR
            int ret = ::ibv_modify_qp(qp_, &attr,
                IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU |
                IBV_QP_DEST_QPN | IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER);
            
            if (ret != 0)
                throw ibv_error();
        }
        
        void modify_qp_rtr_to_rts() {
            ::ibv_qp_attr attr = ::ibv_qp_attr();
            attr.qp_state      = IBV_QPS_RTS;
            attr.timeout       = 0; // Arbitary from 0 to 31 (TODO: Is it true?)
            attr.retry_cnt     = 7; // Arbitary from 0 to 7
            attr.rnr_retry     = 7; // TODO
            attr.sq_psn        = 0; // Arbitary
            attr.max_rd_atomic = 0; // TODO : Usually 0 ?
            
            // RTR to RTS
            int ret = ibv_modify_qp(qp_, &attr,
                IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
                IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC);
            
            if (ret != 0)
                throw ibv_error();
        }
        
        
    public:
        bool try_write_async(mgbase::uint64_t wr_id, mgbase::uint64_t laddr, mgbase::uint32_t lkey, mgbase::uint64_t raddr, mgbase::uint32_t rkey, std::size_t size_in_bytes) {
            ::ibv_sge sge = ::ibv_sge();
            sge.addr   = laddr;
            sge.length = size_in_bytes;
            sge.lkey   = lkey;
            
            ::ibv_send_wr wr = ::ibv_send_wr();
            wr.wr_id               = wr_id;
            wr.next                = MGBASE_NULLPTR;
            wr.sg_list             = &sge;
            wr.num_sge             = 1;
            wr.opcode              = IBV_WR_RDMA_WRITE;
            wr.send_flags          = 0; // TODO
            wr.wr.rdma.remote_addr = raddr;
            wr.wr.rdma.rkey        = rkey;
            
            ::ibv_send_wr* bad_wr;
            int err = ::ibv_post_send(qp_, &wr, &bad_wr);
            if (err == 0)
                return true;
            else if (err == ENOMEM)
                return false;
            else
                throw ibv_error();
        }
    
    private:
        ::ibv_qp* qp_;
    };
    
public:
    void initialize(int* argc, char*** argv) {
        int provided;
        ::MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
        
        int size, rank;
        ::MPI_Comm_size(MPI_COMM_WORLD, &size);
        ::MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        
        current_process_id_  = static_cast<process_id_t>(rank);
        number_of_processes_ = static_cast<index_t>(size);
        
        conns_          = new connection[number_of_processes_];
        local_qp_nums_  = new mgbase::uint32_t[number_of_processes_];
        remote_qp_nums_ = new mgbase::uint32_t[number_of_processes_];
        lids_           = new mgbase::uint16_t[number_of_processes_];
        
        init_ibv();
    }
    
    void finalize() {
        ::MPI_Finalize();
        
        delete[] conns_;
        delete[] local_qp_nums_;
        delete[] remote_qp_nums_;
        delete[] lids_;
    }
    
    bool try_write_async(
        mgbase::uint64_t laddr
    ,   mgbase::uint32_t lkey
    ,   mgbase::uint64_t raddr
    ,   mgbase::uint32_t rkey
    ,   std::size_t      size_in_bytes
    ,   process_id_t     dest_proc
    ,   notifier_t       on_complete
    )
    {
        request* req = alloc_request();
        
        if (conns_[dest_proc].try_write_async(req->wr_id(), laddr, lkey, raddr, rkey, size_in_bytes)) {
            return true;
        }
        else {
            free_request(req);
            return false;
        }
    }
    
    ::ibv_mr* register_memory(void* buf, std::size_t size_in_bytes) {
        ::ibv_mr* mr = ibv_reg_mr(pd_, buf, size_in_bytes, 
            IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
            IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC);
        
        return mr;
    }
    
private:
    class request {
    public:
        mgbase::uint32_t wr_id();
    };
    
    mgbase::uint64_t new_id();
    
    request* alloc_request();
    void free_request(request*);
    
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
        for (process_id_t proc = 0; proc < number_of_processes_; ++proc)
            conns_[proc].create(cq_, pd_);
    }
    
    void query_device() {
        int ret = ibv_query_device(context_, &device_attr_);
        if (ret != 0)
            throw ibv_error();
    }
    
    void query_port() {
        // TODO: Port number is fixed to 0
        int ret = ibv_query_port(context_, 0, &port_attr_);
        if (ret != 0)
            throw ibv_error();
    }
    
    void exchange_qp_nums() {
        for (process_id_t proc = 0; proc < number_of_processes_; ++proc)
            local_qp_nums_[proc] = conns_[proc].get_qp_num();
        
        int ret = MPI_Alltoall(
            local_qp_nums_ , sizeof(mgbase::uint32_t), MPI_BYTE,
            remote_qp_nums_, sizeof(mgbase::uint32_t), MPI_BYTE,
            MPI_COMM_WORLD
        );
        if (ret != MPI_SUCCESS)
            throw ibv_error();
    }
    
    void exchange_lids() {
        int ret = MPI_Allgather(
            &port_attr_.lid, sizeof(mgbase::uint16_t), MPI_BYTE,
            lids_          , sizeof(mgbase::uint16_t), MPI_BYTE,
            MPI_COMM_WORLD
        );
        if (ret != MPI_SUCCESS)
            throw ibv_error();
    }
    
    void modify_qp() {
        for (process_id_t proc = 0; proc < number_of_processes_; ++proc)
            conns_[proc].start(remote_qp_nums_[proc], lids_[proc], device_attr_);
    }
    
    ::ibv_context*    context_;
    ::ibv_pd*         pd_;
    ::ibv_cq*         cq_;
    connection*       conns_;
    
    ::ibv_device_attr device_attr_;
    ::ibv_port_attr   port_attr_;
    mgbase::uint32_t* local_qp_nums_;
    mgbase::uint32_t* remote_qp_nums_;
    mgbase::uint16_t* lids_;
    
    process_id_t current_process_id_;
    index_t number_of_processes_;
};

com_ibv g_com;

struct region_key_ibv {
    mgbase::uint64_t addr;
    mgbase::uint32_t rkey;
};

struct remote_region_ibv {
    mgbase::uint64_t addr;
    mgbase::uint32_t rkey;
};

}

void initialize(int* argc, char*** argv) {
    g_com.initialize(argc, argv);
}

void finalize() {
    g_com.finalize();
}

bool try_write_async(
    local_address_t                local_address
,   remote_address_t               remote_address
,   index_t                        size_in_bytes
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
)
{
    const remote_region_ibv* const remote_region = reinterpret_cast<const remote_region_ibv*>(&remote_address.region);
    const ::ibv_mr* local_mr = reinterpret_cast<const ::ibv_mr*>(local_address.region.local);
    
    return g_com.try_write_async(
        reinterpret_cast<mgbase::uint64_t>(local_mr->addr) + local_address.offset,
        local_mr->lkey,
        remote_region->addr + remote_address.offset,
        remote_region->rkey,
        size_in_bytes,
        dest_proc,
        on_complete
    );
}

local_region_t register_region(
    void*                          local_pointer
,   index_t                        size_in_bytes
) {
    ibv_mr* mr = g_com.register_memory(local_pointer, size_in_bytes);
    
    local_region_t region;
    region.local = reinterpret_cast<mgbase::uint64_t>(mr);
    region_key_ibv* region_key = reinterpret_cast<region_key_ibv*>(&region.key);
    region_key->rkey = mr->rkey;
    region_key->addr = reinterpret_cast<mgbase::uint64_t>(mr->addr);
    return region;
}

remote_region_t use_remote_region(
    process_id_t                   /*proc_id*/
,   region_key_t                   key
,   index_t                        /*size_in_bytes*/
) {
    const region_key_ibv* key_ibv = reinterpret_cast<const region_key_ibv*>(&key);
    remote_region_t region;
    remote_region_ibv* region_ibv = reinterpret_cast<remote_region_ibv*>(&region);
    region_ibv->addr = key_ibv->addr;
    region_ibv->rkey = key_ibv->rkey;
    return region;
}

}

