
#include "unittest.hpp"
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>
#include <menps/mecom2/coll/mpi/mpi_coll.hpp>
#include <menps/mecom2/rma/alltoall_buffer.hpp>
#include <menps/mefdn/coro/klt_worker.hpp>
#include <menps/mefdn/coro/sfc.hpp>
#include <menps/mecom2/com/com_worker.hpp>

#ifdef MEDEV2_DEVICE_UCX_ENABLED
#include <menps/mecom2/rma/ucp/ucp_rma.hpp>

//#include <menps/mecom2/com/uct/uct_worker_set.hpp>
#include <menps/mecom2/rma/uct/uct_rma.hpp>
#endif

TEST(Rma, Basic)
{
    /*const*/ auto win =
        g_mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    g_mi->win_lock_all({ 0, win });
    
    auto rma = mecom2::make_mpi_rma(*g_mi, win, MPI_COMM_WORLD);
    
    {
        const auto arr = rma->make_unique<int []>(1);
        
        auto p = arr.get();
        
        g_mi->broadcast({ &p, sizeof(p), 0, MPI_COMM_WORLD });
        
        {
            auto h = rma->make_handle();
            
            int x = 123;
            h.write_nb(0, p, &x, 1);
            
            h.flush();
        }
        
        ASSERT_EQ(123, *p);
        
        g_mi->win_unlock_all({ win });
    }
    
    g_mi->win_free(&win);
}


namespace /*unnamed*/ {

template <typename A2Abuffer, typename Rma>
void do_alltoall_test(Rma& rma) {
    auto coll = mecom2::make_mpi_coll(*g_mi, MPI_COMM_WORLD);
    
    using proc_id_type = mecom2::mpi_coll::proc_id_type;
    const auto cur_proc = coll.this_proc_id();
    const auto num_procs = coll.get_num_procs();
    
    A2Abuffer buf;
    buf.coll_make(rma, coll, num_procs);
    
    #if 0
    // 1st
    {
        auto h = rma.make_handle();
        for (proc_id_type proc = 0; proc < coll.get_num_procs(); ++proc) {
            const int x = cur_proc + 1;
            h.write_nb(proc, buf.remote(proc, cur_proc), &x, 1);
        }
        h.flush();
    }
    
    coll.barrier();
    
    for (proc_id_type proc = 0; proc < coll.get_num_procs(); ++proc) {
        const int x = * buf.local(proc);
        ASSERT_EQ(proc + 1, x);
    }
    
    coll.barrier();
    #endif
    
    // 2nd
    {
        for (proc_id_type proc = 0; proc < coll.get_num_procs(); ++proc) {
            const int x = cur_proc + 2;
            /*auto x = rma.template make_private_uninitialized<int []>(1);
            x[0] = cur_proc + 2;*/
            rma.buf_write(proc, buf.remote(proc, cur_proc), &x, 1);
        }
    }
    
    coll.barrier();
    
    for (proc_id_type proc = 0; proc < coll.get_num_procs(); ++proc) {
        const int x = * buf.local(proc);
        ASSERT_EQ(proc + 2, x);
    }
}

} // unnamed namespace

TEST(Rma, AlltoallMpi)
{
    const auto win =
        g_mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    g_mi->win_lock_all({ 0, win });
    
    auto rma = mecom2::make_mpi_rma(*g_mi, win, MPI_COMM_WORLD);
    
    do_alltoall_test<mecom2::alltoall_buffer<mecom2::mpi_rma, int>>(*rma);
}

#ifdef MEDEV2_DEVICE_UCX_ENABLED

TEST(Rma, AlltoallUcp)
{
    auto coll = mecom2::make_mpi_coll(*g_mi, MPI_COMM_WORLD);
    
    using mecom2::rma_ucp_policy;
    rma_ucp_policy::ucp_facade_type uf;
    
    auto conf = rma_ucp_policy::config_type::read(uf, nullptr, nullptr);
    
    ucp_params ctx_params = ucp_params();
    ctx_params.field_mask = UCP_PARAM_FIELD_FEATURES;
    ctx_params.features   = UCP_FEATURE_RMA;
    
    auto ctx = rma_ucp_policy::context_type::init(uf, &ctx_params, conf.get());
    
    ucp_worker_params_t wk_params = ucp_worker_params_t();
    auto wk_set = mecom2::make_ucp_worker_set(uf, ctx, wk_params, coll);
    
    auto rma = mecom2::make_ucp_rma(uf, ctx, *wk_set);
    
    do_alltoall_test<mecom2::alltoall_buffer<mecom2::ucp_rma, int>>(*rma);
}

TEST(Rma, AlltoallUct)
{
    auto coll = mecom2::make_mpi_coll(*g_mi, MPI_COMM_WORLD);
    
    
    // TODO
    const char tl_name[] = "rc_mlx5";
    const char dev_name[] = "mlx5_0:1";
    
    #if 0
    using mecom2::rma_uct_policy;
    rma_uct_policy::uct_facade_type uf;
    
    auto md = rma_uct_policy::open_md(uf, tl_name, dev_name);
    
    auto iface_conf =
        rma_uct_policy::iface_config_type::read(
            uf, md.get(), tl_name, nullptr, nullptr);
    
    uct_iface_params_t iface_params = uct_iface_params_t();
    iface_params.open_mode = UCT_IFACE_OPEN_MODE_DEVICE;
    iface_params.mode.device.tl_name = tl_name;
    iface_params.mode.device.dev_name = dev_name;
    iface_params.stats_root = ucs_stats_get_root();
    iface_params.rx_headroom = 0;
    
    auto wk_set =
        mecom2::make_uct_worker_set(uf, md, iface_conf.get(), &iface_params, coll);
    
    auto rma = mecom2::make_uct_rma(uf, md, *wk_set);
    #endif
    
    auto rma_res = mecom2::make_uct_rma_resource(tl_name, dev_name, coll);
    
    do_alltoall_test<mecom2::alltoall_buffer<mecom2::uct_rma, int>>(*rma_res->rma);
    
    #if 0
    
    auto conf = rma_uct_policy::config_type::read(uf, nullptr, nullptr);
    
    uct_params ctx_params = uct_params();
    ctx_params.field_mask = uct_PARAM_FIELD_FEATURES;
    ctx_params.features   = uct_FEATURE_RMA;
    
    auto ctx = rma_uct_policy::context_type::init(uf, &ctx_params, conf.get());
    
    uct_worker_params_t wk_params = uct_worker_params_t();
    auto wk_set = mecom2::make_uct_worker_set(uf, ctx, wk_params, coll);
    
    auto rma = mecom2::make_uct_rma(uf, ctx, *wk_set);
    
    do_alltoall_test<mecom2::uct_alltoall_buffer<int>>(*rma);
    #endif
}


#endif


TEST(Rma, Cas)
{
    const auto win =
        g_mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    g_mi->win_lock_all({ 0, win });
    
    auto rma = mecom2::make_mpi_rma(*g_mi, win, MPI_COMM_WORLD);
    auto coll = mecom2::make_mpi_coll(*g_mi, MPI_COMM_WORLD);   
    
    const auto cur_proc = coll.this_proc_id();
    const auto num_procs = coll.get_num_procs();
    
    mecom2::alltoall_buffer<mecom2::mpi_rma, mefdn::uint64_t> buf;
    buf.coll_make(*rma, coll, 1);
    
    if (cur_proc == 0)
        *buf.local(0) = 0;
    
    while (true) {
        const mefdn::uint64_t expected = cur_proc;
        const mefdn::uint64_t desired  = cur_proc + 1;
        
        mefdn::uint64_t result = 0;
        
        rma->compare_and_swap(
            0
        ,   buf.remote(0, 0)
        ,   &expected
        ,   &desired
        ,   &result
        );
        
        if (result == expected)
            break;
    }
    
    coll.barrier();
    
    if (cur_proc == 0)
        ASSERT_EQ(num_procs, *buf.local(0));
}

struct coro_read_vars {
    using rma_itf_type = menps::mecom2::mpi_rma;
    using coll_itf_type = menps::mecom2::mpi_coll;
    using rptr_type = rma_itf_type::remote_ptr<mefdn::uint64_t>;
    using lptr_type = rma_itf_type::local_ptr<mefdn::uint64_t>;
    using a2a_buf_type =
        mecom2::alltoall_buffer<mecom2::mpi_rma, mefdn::uint64_t>;
    
    rma_itf_type&   rma;
    coll_itf_type&  coll;
    a2a_buf_type&   a2a_buf;
    lptr_type       lptr;
};

template <typename F>
struct coro_read_read : F
{
private:
    using rma_itf_type = coro_read_vars::rma_itf_type;
    using proc_id_type = typename rma_itf_type::proc_id_type;
    
public:
    using result_type = void;
    using children = typename F::template define_children<rma_itf_type::coro_read>;
    
    explicit coro_read_read(const proc_id_type src_proc, coro_read_vars& vars)
        : src_proc_(src_proc)
        , vars_(vars)
    { }
    
    template <typename C>
    struct start : C {
        typename C::return_type operator() () {
            auto& fr = this->frame();
            
            return this->template call<finish, rma_itf_type::coro_read>(
                fr.vars_.rma
            ,   fr.src_proc_
            ,   fr.vars_.a2a_buf.remote(fr.src_proc_, 0)
            ,   (fr.vars_.lptr + fr.src_proc_)
            ,   1
            );
        }
    };
    
private:
    template <typename C>
    struct finish : C {
        typename C::return_type operator() () {
            return this->set_return();
        }
    };
    
    proc_id_type    src_proc_;
    coro_read_vars& vars_;
};

template <typename F>
struct coro_read_main : F
{
    using result_type = void;
    
    explicit coro_read_main(coro_read_vars& vars)
        : vars_(vars)
    { }
    
    template <typename C>
    struct start : C {
        typename C::return_type operator() () {
            auto& fr = this->frame();
            const auto num_procs = fr.vars_.coll.get_num_procs();
            
            return this->template parallel_for<finish, coro_read_read>(
                0, num_procs, 1, fr.vars_);
        }
    };
    
private:
    template <typename C>
    struct finish : C {
        typename C::return_type operator() () {
            return this->set_return();
        }
    };
    
    coro_read_vars& vars_;
};

TEST(Rma, CoroRead)
{
    /*const*/ auto win =
        g_mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    g_mi->win_lock_all({ 0, win });
    
    auto rma = mecom2::make_mpi_rma(*g_mi, win, MPI_COMM_WORLD);
    auto coll = mecom2::make_mpi_coll(*g_mi, MPI_COMM_WORLD);
    
    mecom2::alltoall_buffer<mecom2::mpi_rma, mefdn::uint64_t> a2a_buf;
    a2a_buf.coll_make(*rma, coll, 1);
    
    const auto num_procs = coll.get_num_procs();
    const auto this_proc = coll.this_proc_id();
    
    *a2a_buf.local(0) = (this_proc+1) * 10;
    
    const auto lbuf = rma->make_unique<mefdn::uint64_t []>(num_procs);
    
    coro_read_vars vars{ *rma, coll, a2a_buf, lbuf.get() };
    
    using worker_type = mecom2::com_worker<mefdn::klt_worker>;
    worker_type wk;
    mefdn::call_sfc<coro_read_main>(wk, vars);
    
    for (int proc = 0; proc < num_procs; ++proc) {
        ASSERT_EQ((proc+1)*10, lbuf.get()[proc]);
    }
    
    g_mi->win_unlock_all({ win });
    
    g_mi->win_free(&win);
}

