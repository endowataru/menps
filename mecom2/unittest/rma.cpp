
#include "unittest.hpp"
#include <menps/mecom2/rma/mpi/mpi_rma.hpp>

TEST(Rma, Basic)
{
    struct conf {
        medev2::mpi::direct_requester& req;
        MPI_Win win;
    };
    
    const auto win =
        g_mi->win_create_dynamic({ MPI_INFO_NULL, MPI_COMM_WORLD });
    
    g_mi->win_lock_all({ 0, win });
    
    mecom2::mpi_rma r{ conf{ *g_mi, win } };
    
    const auto arr = r.make_unique<int []>(1);
    
    auto p = arr.get();
    
    g_mi->broadcast({ &p, sizeof(p), 0, MPI_COMM_WORLD });
    
    {
        auto h = r.make_handle();
        
        int x = 123;
        h.write(0, p, &x, 1);
        
        h.flush();
    }
    
    ASSERT_EQ(123, *p);
    
    //g_mi->win_free(win);
    
    #if 0
    #endif
}


