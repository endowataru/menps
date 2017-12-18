
#include "unittest.hpp"
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>
#include <menps/mecom2/coll/mpi/mpi_coll.hpp>
#include <menps/mecom2/rma/mpi/mpi_alltoall_buffer.hpp>

TEST(Rma, Basic)
{
    /*const*/ auto win =
        g_mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    g_mi->win_lock_all({ 0, win });
    
    auto rma = mecom2::make_mpi_rma(*g_mi, win);
    
    {
        const auto arr = rma.make_unique<int []>(1);
        
        auto p = arr.get();
        
        g_mi->broadcast({ &p, sizeof(p), 0, MPI_COMM_WORLD });
        
        {
            auto h = rma.make_handle();
            
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
    
    auto rma = mecom2::make_mpi_rma(*g_mi, win);
    auto coll = mecom2::make_mpi_coll(*g_mi, MPI_COMM_WORLD);
    
    using process_id_type = /*mecom2::mpi_coll::process_id_type*/int;
    const auto cur_proc = coll.current_process_id();
    const auto num_procs = coll.number_of_processes();
    
    mecom2::mpi_alltoall_buffer<int> buf;
    buf.coll_make(rma, coll, num_procs);
    
    // 1st
    {
        auto h = rma.make_handle();
        for (process_id_type proc = 0; proc < coll.number_of_processes(); ++proc) {
            const int x = cur_proc + 1;
            h.write_nb(proc, buf.remote(proc, cur_proc), &x, 1);
        }
        h.flush();
    }
    
    coll.barrier();
    
    for (process_id_type proc = 0; proc < coll.number_of_processes(); ++proc) {
        const int x = * buf.local(proc);
        ASSERT_EQ(proc + 1, x);
    }
    
    coll.barrier();
    
    // 2nd
    {
        for (process_id_type proc = 0; proc < coll.number_of_processes(); ++proc) {
            const int x = cur_proc + 2;
            rma.write(proc, buf.remote(proc, cur_proc), &x, 1);
        }
    }
    
    coll.barrier();
    
    for (process_id_type proc = 0; proc < coll.number_of_processes(); ++proc) {
        const int x = * buf.local(proc);
        ASSERT_EQ(proc + 2, x);
    }
}

TEST(Rma, Cas)
{
    const auto win =
        g_mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    g_mi->win_lock_all({ 0, win });
    
    auto rma = mecom2::make_mpi_rma(*g_mi, win);
    auto coll = mecom2::make_mpi_coll(*g_mi, MPI_COMM_WORLD);   
    
    using process_id_type = /*mecom2::mpi_coll::process_id_type*/int;
    const auto cur_proc = coll.current_process_id();
    const auto num_procs = coll.number_of_processes();
    
    mecom2::mpi_alltoall_buffer<mefdn::uint64_t> buf;
    buf.coll_make(rma, coll, 1);
    
    if (cur_proc == 0)
        *buf.local(0) = 0;
    
    while (true) {
        const mefdn::uint64_t expected = cur_proc;
        const mefdn::uint64_t desired  = cur_proc + 1;
        
        const auto result =
            rma.compare_and_swap(
                0
            ,   buf.remote(0, 0)
            ,   expected
            ,   desired
            );
        
        if (result == expected)
            break;
    }
    
    coll.barrier();
    
    if (cur_proc == 0)
        ASSERT_EQ(num_procs, *buf.local(0));
    
    #if 0
    for (process_id_type proc = 0; proc < coll.number_of_processes(); ++proc)
    {
        rma.compare_and_swap(
            
        );
    }
    #endif
    
}

