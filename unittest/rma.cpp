
#include "unittest.hpp"
#include <vector>

class RmaBasic
    : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        if (mgcom::current_process_id() == 0) {
            lptr = mgcom::rma::allocate<int>(1);
            *lptr = 123;
        }
        
        mgcom::collective::broadcast(0, &lptr, 1);
        
        rptr = mgcom::rma::use_remote_pointer(0, lptr);
        
        buf = mgcom::rma::allocate<int>(1);
    }
    
    mgcom::rma::local_pointer<int> lptr;
    mgcom::rma::remote_pointer<int> rptr;
    mgcom::rma::local_pointer<int> buf;
};

TEST_F(RmaBasic, Get)
{
    *buf = 0;
    
    mgcom::rma::remote_read(0, rptr, buf, 1);
    
    const int val = *buf;
    ASSERT_EQ(123, val);
}

TEST_F(RmaBasic, Put)
{
    if (mgcom::current_process_id() == 0) {
        *buf = 234;
        
        mgcom::rma::remote_write(0, rptr, buf, 1);
        
        const int val = *lptr;
        ASSERT_EQ(234, val);
    }
    
    mgcom::collective::barrier();
    
    *buf = 0;
    mgcom::rma::remote_read(0, rptr, buf, 1);
    
    const int val = *buf;
    ASSERT_EQ(234, val);
}

TEST_F(RmaBasic, ConcurrentGet)
{
    mgbase::atomic<mgbase::uint64_t> count = MGBASE_ATOMIC_VAR_INIT(0);
    typedef std::vector< mgcom::rma::local_pointer<int> > vec_type;
    vec_type ptrs;
    
    for (mgbase::uint64_t i = 0; i < 100; ++i) {
        ptrs.push_back(mgcom::rma::allocate<int>());
    }
    
    for (vec_type::iterator itr = ptrs.begin(); itr != ptrs.end(); ++itr) {
        mgcom::rma::remote_read_async(
            0
        ,   rptr
        ,   *itr
        ,   1
        ,   mgbase::make_operation_fetch_add_release(&count, static_cast<mgbase::uint64_t>(1))
        );
    }
    
    while (true) {
        const mgbase::uint64_t x = count.load(mgbase::memory_order_acquire);
        if (x == ptrs.size())
            break;
    }
    
    for (vec_type::iterator itr = ptrs.begin(); itr != ptrs.end(); ++itr) {
        ASSERT_EQ(123, **itr);
    }
}

