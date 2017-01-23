
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

#if ! (MGBASE_COMPILER_GCC_VERSION < 40500)
// ICE on GCC 4.4

namespace /*unnamed*/ {

mgctx::transfer<int*> f_restore_2(int* d)
{
    return { d };
}

mgctx::transfer<int*> f_restore(mgctx::context<int*> ctx, int* d)
{
    mgctx::restore_context(
        ctx
    ,   MGBASE_NONTYPE(&f_restore_2)
    ,   d
    );
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

#endif

namespace /*unnamed*/ {

mgctx::context<int*> g_ctx_swap;

mgctx::transfer<int*> f_swap_3(int* d)
{
    return { d };
}

#ifdef MGBASE_COMPILER_CLANG
MGBASE_NORETURN
#endif
void f_swap_2(mgctx::transfer<int*> tr)
{
    mgctx::restore_context(
        g_ctx_swap
    ,   MGBASE_NONTYPE(&f_swap_3)
    ,   tr.p0
    );
}

mgctx::transfer<int*> f_swap(mgctx::context<int*> ctx, int* d)
{
    g_ctx_swap = ctx;
    
    return { d };
}

} // unnamed namespace

TEST(Context, Swap)
{
    const mgbase::size_t stack_size = 4096;
    
    mgbase::unique_ptr<char []> st1{ new char[stack_size] };
    
    const auto ctx =
        mgctx::make_context(
            st1.get() + stack_size
        ,   stack_size
        ,   MGBASE_NONTYPE(&f_swap_2)
        );
    
    const auto x =
        mgctx::swap_context(
            ctx
        ,   MGBASE_NONTYPE(&f_swap)
        ,   reinterpret_cast<int*>(111)
        );
    
    ASSERT_EQ(111, reinterpret_cast<uip>(x.p0));
}

