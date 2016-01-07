
#include "unittest.hpp"

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
    
    mgcom::rma::remote_read_cb cb;
    mgcom::rma::remote_read_nb(cb, 0, rptr, buf, 1)
        .wait();
    
    ASSERT_EQ(*buf, 123);
}

TEST_F(RmaBasic, Put)
{
    if (mgcom::current_process_id() == 1) {
        *buf = 234;
        
        mgcom::rma::remote_write_cb cb;
        mgcom::rma::remote_write_nb(cb, 0, rptr, buf, 1)
            .wait();
    }
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() == 0) {
        ASSERT_EQ(*lptr, 234);
    }
}

