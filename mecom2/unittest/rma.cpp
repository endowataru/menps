
#include "unittest.hpp"
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>
#include <menps/mecom2/coll/mpi/mpi_coll.hpp>
#include <menps/mecom2/rma/mpi/mpi_alltoall_buffer.hpp>

TEST(Rma, Basic)
{
    /*const*/ auto win =
        g_mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    g_mi->win_lock_all({ 0, win });
    
    struct conf {
        medev2::mpi::direct_requester& req;
        MPI_Win win;
    };
    mecom2::mpi_rma r{ conf{ *g_mi, win } };
    
    {
        const auto arr = r.make_unique<int []>(1);
        
        auto p = arr.get();
        
        g_mi->broadcast({ &p, sizeof(p), 0, MPI_COMM_WORLD });
        
        {
            auto h = r.make_handle();
            
            int x = 123;
            h.write_nb(0, p, &x, 1);
            
            h.flush();
        }
        
        ASSERT_EQ(123, *p);
        
        g_mi->win_unlock_all({ win });
    }
    
    g_mi->win_free(&win);
}

TEST(Rma, Alltoall)
{
    const auto win =
        g_mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    g_mi->win_lock_all({ 0, win });
    
    struct rma_conf {
        medev2::mpi::direct_requester& req;
        MPI_Win win;
    };
    mecom2::mpi_rma r{ rma_conf{ *g_mi, win } };
    
    struct coll_conf {
        medev2::mpi::direct_requester& req;
        MPI_Comm comm;
    };
    mecom2::mpi_coll c{ coll_conf{ *g_mi, MPI_COMM_WORLD} };
    
    mecom2::mpi_alltoall_buffer<int> buf;
    buf.coll_make(r, c, 10);
    
    using process_id_type = /*mecom2::mpi_coll::process_id_type*/int;
    const auto cur_proc = c.current_process_id();
    const auto num_procs = c.number_of_processes();
    
    // 1st
    {
        auto h = r.make_handle();
        for (process_id_type proc = 0; proc < c.number_of_processes(); ++proc) {
            const int x = cur_proc + 1;
            h.write_nb(proc, buf.remote(proc, cur_proc), &x, 1);
        }
        h.flush();
    }
    
    c.barrier();
    
    for (process_id_type proc = 0; proc < c.number_of_processes(); ++proc) {
        const int x = * buf.local(proc);
        ASSERT_EQ(proc + 1, x);
    }
    
    // 2nd
    {
        for (process_id_type proc = 0; proc < c.number_of_processes(); ++proc) {
            const int x = cur_proc + 2;
            r.write(proc, buf.remote(proc, cur_proc), &x, 1);
        }
    }
    
    c.barrier();
    
    for (process_id_type proc = 0; proc < c.number_of_processes(); ++proc) {
        const int x = * buf.local(proc);
        ASSERT_EQ(proc + 2, x);
    }
}




