
#include "unittest.hpp"
#include <menps/mecom2/rpc/mpi/mpi_rpc.hpp>
#include <menps/mefdn/thread.hpp>
#include <menps/mefdn/atomic.hpp>

using namespace menps;

struct test_handler {
    static const mefdn::size_t handler_id = 1;
    
    typedef int*    request_type;
    typedef int     reply_type;
    
    template <typename ServerCtx>
    typename ServerCtx::return_type operator() (ServerCtx& sc) const
    {
        const auto src_proc = sc.src_proc();
        const auto& rqst = sc.request();
        *rqst += 10;
        
        auto rply = sc.make_reply();
        *rply = static_cast<int>(src_proc);
        return rply;
    }
};

TEST(Rpc, Base)
{
    struct conf {
        medev2::mpi::default_direct_mpi_itf::mpi_facade_type& mi;
        MPI_Comm comm;
        mefdn::size_t max_num_handlers;
        int reply_tag_start;
        int reply_tag_end;
        int request_tag;
    };
    
    mecom2::mpi_rpc r{ conf{ *g_mi, MPI_COMM_WORLD, 1024 , 1, 1000, 0} };
    
    r.add_handler(test_handler());
    
    mefdn::atomic<bool> b{false};
    
    mefdn::thread th([&] {
        while (!b.load()) { 
            if (!r.try_progress())
                mefdn::this_thread::yield();
        }
    });
    int x = 1000;
    int* px = &x;
    
    g_mi->bcast({ &px, sizeof(px), MPI_BYTE, 0, MPI_COMM_WORLD });
    
    r.call<test_handler>(0, px);
    
    g_mi->barrier({ MPI_COMM_WORLD });
    
    b = true;
    th.join();
    
    std::cout << x;
    //ASSERT_EQ(1000 + static_cast<int>(mecom::number_of_processes()) * 10, x);
}


