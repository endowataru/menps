
#include "unittest.hpp"
#include <mgctx.hpp>
#include <mgbase/unique_ptr.hpp>

namespace /*unnamed*/ {

typedef mgbase::uintptr_t   uip;

mgctx::transfer<int*> f_return(mgctx::context<int*>, int* d)
{
    return { d };
}

} // unnamed namespace

TEST(Context, Return)
{
    const mgbase::size_t stack_size = 4096;
    
    mgbase::unique_ptr<char []> st1{ new char[stack_size] };
    
    const auto x =
        mgctx::save_context(
            st1.get() + stack_size
        ,   stack_size
        ,   MGBASE_NONTYPE(&f_return)
        ,   reinterpret_cast<int*>(111)
        );
    
    ASSERT_EQ(111, reinterpret_cast<uip>(x.p0));
}

namespace /*unnamed*/ {

mgctx::transfer<int*> f_restore(mgctx::context<int*> ctx, int* d)
{
    mgctx::restore_context(ctx, { d });
}

} // unnamed namespace

TEST(Context, Restore)
{
    const mgbase::size_t stack_size = 4096;
    
    mgbase::unique_ptr<char []> st1{ new char[stack_size] };
    
    const auto x =
        mgctx::save_context(
            st1.get() + stack_size
        ,   stack_size
        ,   MGBASE_NONTYPE(&f_restore)
        ,   reinterpret_cast<int*>(111)
        );
    
    ASSERT_EQ(111, reinterpret_cast<uip>(x.p0));
}

