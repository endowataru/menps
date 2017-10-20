
#include "unittest.hpp"
#include <vector>

namespace ult = mecom::ult;

class RmaBasic
    : public ::testing::Test
{
protected:
    virtual void SetUp() MEFDN_OVERRIDE
    {
        if (mecom::current_process_id() == 0) {
            lptr = mecom::rma::allocate<int>(1);
            *lptr = 123;
        }
        
        mecom::collective::broadcast(0, &lptr, 1);
        
        rptr = mecom::rma::use_remote_ptr(0, lptr);
        
        buf = mecom::rma::allocate<int>(1);
    }
    
    virtual void TearDown() MEFDN_OVERRIDE
    {
        mecom::collective::barrier();
        
        if (mecom::current_process_id() == 0) {
            mecom::rma::deallocate(lptr);
        }
        mecom::rma::deallocate(buf);
    }
    
    mecom::rma::local_ptr<int> lptr;
    mecom::rma::remote_ptr<int> rptr;
    mecom::rma::local_ptr<int> buf;
};

TEST_F(RmaBasic, Get)
{
    *buf = 0;
    
    mecom::rma::read(0, rptr, buf, 1);
    
    const int val = *buf;
    ASSERT_EQ(123, val);
    
    mecom::collective::barrier();
}

TEST_F(RmaBasic, Put)
{
    if (mecom::current_process_id() == 0) {
        *buf = 234;
        
        mecom::rma::write(0, rptr, buf, 1);
        
        const int val = *lptr;
        ASSERT_EQ(234, val);
    }
    
    mecom::collective::barrier();
    
    *buf = 0;
    mecom::rma::read(0, rptr, buf, 1);
    
    const int val = *buf;
    ASSERT_EQ(234, val);
    
    mecom::collective::barrier();
}

TEST_F(RmaBasic, ConcurrentGet)
{
    mefdn::atomic<mefdn::uint64_t> count{0};
    typedef std::vector< mecom::rma::local_ptr<int> > vec_type;
    vec_type ptrs;
    
    for (mefdn::uint64_t i = 0; i < 100; ++i) {
        ptrs.push_back(mecom::rma::allocate<int>());
    }
    
    for (vec_type::iterator itr = ptrs.begin(); itr != ptrs.end(); ++itr) {
        auto r =
            mecom::rma::async_read(
                0
            ,   rptr
            ,   *itr
            ,   1
            ,   mefdn::make_callback_fetch_add_release(&count, MEFDN_NONTYPE(1u))
            );
        
        if (r.is_ready()) {
            count.fetch_add(1);
        }
    }
    
    while (true) {
        const mefdn::uint64_t x = count.load(mefdn::memory_order_acquire);
        if (x == ptrs.size())
            break;
        
        ult::this_thread::yield();
    }
    
    for (vec_type::iterator itr = ptrs.begin(); itr != ptrs.end(); ++itr) {
        ASSERT_EQ(123, **itr);
    }
    
    for (auto itr = ptrs.begin(); itr != ptrs.end(); ++itr) {
        mecom::rma::deallocate(*itr);
    }
    
    mecom::collective::barrier();
}

class RmaAtomic
    : public ::testing::Test
{
protected:
    virtual void SetUp() MEFDN_OVERRIDE
    {
        if (mecom::current_process_id() == 0) {
            lptr = mecom::rma::allocate<mecom::rma::atomic_default_t>(1);
            *lptr = 123;
        }
        
        mecom::collective::broadcast(0, &lptr, 1);
        
        rptr = mecom::rma::use_remote_ptr(0, lptr);
        
        buf = mecom::rma::allocate<mecom::rma::atomic_default_t>(1);
        buf2 = mecom::rma::allocate<mecom::rma::atomic_default_t>(1);
        buf3 = mecom::rma::allocate<mecom::rma::atomic_default_t>(1);
    }
    
    virtual void TearDown() MEFDN_OVERRIDE
    {
        mecom::collective::barrier();
        
        if (mecom::current_process_id() == 0) {
            mecom::rma::deallocate(lptr);
        }
        mecom::rma::deallocate(buf);
        mecom::rma::deallocate(buf2);
        mecom::rma::deallocate(buf3);
    }
    
    mecom::rma::local_ptr<mecom::rma::atomic_default_t> lptr;
    mecom::rma::remote_ptr<mecom::rma::atomic_default_t> rptr;
    mecom::rma::local_ptr<mecom::rma::atomic_default_t> buf;
    mecom::rma::local_ptr<mecom::rma::atomic_default_t> buf2;
    mecom::rma::local_ptr<mecom::rma::atomic_default_t> buf3;
};


TEST_F(RmaAtomic, AtomicRead)
{
    const mecom::rma::atomic_default_t some_const = 234;
    
    if (mecom::current_process_id() == 0) {
        *lptr = some_const;
    }
    
    mecom::collective::barrier();
    
    mecom::rma::atomic_default_t dest = 0;
    
    mecom::rma::atomic_read<mecom::rma::atomic_default_t>(
        0
    ,   rptr
    ,   &dest
    );
    
    ASSERT_EQ(some_const, dest);
}

TEST_F(RmaAtomic, AtomicWrite)
{
    const mecom::rma::atomic_default_t some_const = 234;
    
    if (mecom::current_process_id() == 0) {
        *lptr = some_const;
    }
    
    mecom::collective::barrier();
    
    for (mecom::process_id_t proc = 0; proc < mecom::number_of_processes(); ++proc)
    {
        if (proc == mecom::current_process_id()) {
            mecom::rma::atomic_write<mecom::rma::atomic_default_t>(
                0
            ,   rptr
            ,   some_const + proc
            );
        }
        
        mecom::collective::barrier();
        
        if (mecom::current_process_id() == 0) {
            ASSERT_EQ(some_const + proc, *lptr);
        }
        
        mecom::collective::barrier();
    }
}

TEST_F(RmaAtomic, FetchAndAdd)
{
    if (mecom::current_process_id() == 0) {
        *lptr = 1;
    }
    
    mecom::collective::barrier();
    
    const mecom::rma::atomic_default_t diff = 100;
    
    mecom::rma::fetch_and_add<mecom::rma::atomic_default_t>(
        0
    ,   rptr
    ,   diff
    ,   buf2
    );
    
    mecom::collective::barrier();
    
    ASSERT_GE((mecom::number_of_processes() - 1) * diff + 1, *buf2);
    ASSERT_EQ(1, *buf2 % diff);
    
    if (mecom::current_process_id() == 0) {
        mecom::rma::atomic_default_t val = *lptr;
        ASSERT_EQ(mecom::number_of_processes() * diff + 1, val);
    }
    
    mecom::collective::barrier();
}


TEST_F(RmaAtomic, CompareAndSwap)
{
    if (mecom::current_process_id() == 0) {
        *lptr = 1;
    }
    
    mecom::collective::barrier();
    
    *buf = mecom::current_process_id() * 100 + 1;
    *buf2 = mecom::current_process_id() * 100 + 101;
    
    for (mecom::process_id_t proc = 0; proc < mecom::number_of_processes(); ++proc)
    {
        if (mecom::current_process_id() == proc)
        {
            mecom::rma::compare_and_swap<mecom::rma::atomic_default_t>(
                0
            ,   rptr
            ,   *buf
            ,   *buf2
            ,   buf3
            );
        }
        
        mecom::collective::barrier();
        
        if (mecom::current_process_id() == 0)
        {
            ASSERT_EQ(proc * 100 + 101, *lptr);
        }
        
        if (mecom::current_process_id() == proc)
        {
            ASSERT_EQ(*buf3, *buf);
        }
        
        mecom::collective::barrier();
    }
    
    mecom::collective::barrier();
    
    if (mecom::current_process_id() == 0) {
        mecom::rma::atomic_default_t val = *lptr;
        ASSERT_EQ(mecom::number_of_processes() * 100 + 1, val);
    }
    
    mecom::collective::barrier();
}

