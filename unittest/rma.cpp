
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
        
        rptr = mgcom::rma::use_remote_ptr(0, lptr);
        
        buf = mgcom::rma::allocate<int>(1);
    }
    
    mgcom::rma::local_ptr<int> lptr;
    mgcom::rma::remote_ptr<int> rptr;
    mgcom::rma::local_ptr<int> buf;
};

TEST_F(RmaBasic, Get)
{
    *buf = 0;
    
    mgcom::rma::read(0, rptr, buf, 1);
    
    const int val = *buf;
    ASSERT_EQ(123, val);
}

TEST_F(RmaBasic, Put)
{
    if (mgcom::current_process_id() == 0) {
        *buf = 234;
        
        mgcom::rma::write(0, rptr, buf, 1);
        
        const int val = *lptr;
        ASSERT_EQ(234, val);
    }
    
    mgcom::collective::barrier();
    
    *buf = 0;
    mgcom::rma::read(0, rptr, buf, 1);
    
    const int val = *buf;
    ASSERT_EQ(234, val);
}

TEST_F(RmaBasic, ConcurrentGet)
{
    mgbase::atomic<mgbase::uint64_t> count = MGBASE_ATOMIC_VAR_INIT(0);
    typedef std::vector< mgcom::rma::local_ptr<int> > vec_type;
    vec_type ptrs;
    
    for (mgbase::uint64_t i = 0; i < 100; ++i) {
        ptrs.push_back(mgcom::rma::allocate<int>());
    }
    
    for (vec_type::iterator itr = ptrs.begin(); itr != ptrs.end(); ++itr) {
        mgcom::rma::read_async(
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
        
        mgbase::ult::this_thread::yield();
    }
    
    for (vec_type::iterator itr = ptrs.begin(); itr != ptrs.end(); ++itr) {
        ASSERT_EQ(123, **itr);
    }
}

class RmaAtomic
    : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        if (mgcom::current_process_id() == 0) {
            lptr = mgcom::rma::allocate<mgcom::rma::atomic_default_t>(1);
            *lptr = 123;
        }
        
        mgcom::collective::broadcast(0, &lptr, 1);
        
        rptr = mgcom::rma::use_remote_ptr(0, lptr);
        
        buf = mgcom::rma::allocate<mgcom::rma::atomic_default_t>(1);
        buf2 = mgcom::rma::allocate<mgcom::rma::atomic_default_t>(1);
        buf3 = mgcom::rma::allocate<mgcom::rma::atomic_default_t>(1);
    }
    
    mgcom::rma::local_ptr<mgcom::rma::atomic_default_t> lptr;
    mgcom::rma::remote_ptr<mgcom::rma::atomic_default_t> rptr;
    mgcom::rma::local_ptr<mgcom::rma::atomic_default_t> buf;
    mgcom::rma::local_ptr<mgcom::rma::atomic_default_t> buf2;
    mgcom::rma::local_ptr<mgcom::rma::atomic_default_t> buf3;
};


TEST_F(RmaAtomic, FetchAndAdd)
{
    if (mgcom::current_process_id() == 0) {
        *lptr = 1;
    }
    
    mgcom::collective::barrier();
    
    *buf = 100;
    
    mgcom::rma::fetch_and_add(
        0
    ,   rptr
    ,   *buf
    ,   buf2
    );
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() == 0) {
        mgcom::rma::atomic_default_t val = *lptr;
        ASSERT_EQ(mgcom::number_of_processes() * 100 + 1, val);
    }
}


TEST_F(RmaAtomic, CompareAndSwap)
{
    if (mgcom::current_process_id() == 0) {
        *lptr = 1;
    }
    
    mgcom::collective::barrier();
    
    *buf = mgcom::current_process_id() * 100 + 1;
    *buf2 = mgcom::current_process_id() * 100 + 101;
    
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
    {
        if (mgcom::current_process_id() == proc)
        {
            mgcom::rma::compare_and_swap(
                0
            ,   rptr
            ,   *buf
            ,   *buf2
            ,   buf3
            );
        }
        
        mgcom::collective::barrier();
        
        if (mgcom::current_process_id() == 0)
        {
            ASSERT_EQ(proc * 100 + 101, *lptr);
        }
        
        if (mgcom::current_process_id() == proc)
        {
            ASSERT_EQ(*buf3, *buf);
        }
        
        mgcom::collective::barrier();
    }
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() == 0) {
        mgcom::rma::atomic_default_t val = *lptr;
        ASSERT_EQ(mgcom::number_of_processes() * 100 + 1, val);
    }
}

