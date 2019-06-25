
#include "unittest.hpp"
#include <menps/mectx.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace mefdn = menps::mefdn;
namespace mectx = menps::mectx;

namespace /*unnamed*/ {

typedef mefdn::uintptr_t   uip;

MECTX_SWITCH_FUNCTION
mectx::transfer<int*> f_return(mectx::context<int*>, int* d)
{
    return { d };
}

} // unnamed namespace

TEST(Context, Return)
{
    const mefdn::size_t stack_size = 4096;
    
    mefdn::unique_ptr<char []> st1{ new char[stack_size] };
    
    const auto x =
        mectx::save_context(
            st1.get() + stack_size
        ,   stack_size
        ,   MEFDN_NONTYPE(&f_return)
        ,   reinterpret_cast<int*>(111)
        );
    
    ASSERT_EQ(111, reinterpret_cast<uip>(x.p0));
}

#if ! (MEFDN_COMPILER_GCC_VERSION < 40500)
// ICE on GCC 4.4

namespace /*unnamed*/ {

MECTX_SWITCH_FUNCTION
mectx::transfer<int*> f_restore_2(int* d)
{
    return { d };
}

MECTX_SWITCH_FUNCTION
mectx::transfer<int*> f_restore(mectx::context<int*> ctx, int* d)
{
    mectx::restore_context(
        ctx
    ,   MEFDN_NONTYPE(&f_restore_2)
    ,   d
    );
}

} // unnamed namespace

TEST(Context, Restore)
{
    const mefdn::size_t stack_size = 4096;
    
    mefdn::unique_ptr<char []> st1{ new char[stack_size] };
    
    const auto x =
        mectx::save_context(
            st1.get() + stack_size
        ,   stack_size
        ,   MEFDN_NONTYPE(&f_restore)
        ,   reinterpret_cast<int*>(111)
        );
    
    ASSERT_EQ(111, reinterpret_cast<uip>(x.p0));
}

#endif

namespace /*unnamed*/ {

mectx::context<int*> g_ctx_swap;

MECTX_SWITCH_FUNCTION
mectx::transfer<int*> f_swap_3(int* d)
{
    return { d };
}

#ifdef MEFDN_COMPILER_CLANG
MEFDN_NORETURN
#endif
MECTX_SWITCH_FUNCTION
void f_swap_2(mectx::transfer<int*> tr)
{
    mectx::restore_context(
        g_ctx_swap
    ,   MEFDN_NONTYPE(&f_swap_3)
    ,   tr.p0
    );
}

MECTX_SWITCH_FUNCTION
mectx::transfer<int*> f_swap(mectx::context<int*> ctx, int* d)
{
    g_ctx_swap = ctx;
    
    return { d };
}

} // unnamed namespace

TEST(Context, Swap)
{
    const mefdn::size_t stack_size = 4096;
    
    mefdn::unique_ptr<char []> st1{ new char[stack_size] };
    
    const auto ctx =
        mectx::make_context(
            st1.get() + stack_size
        ,   stack_size
        ,   MEFDN_NONTYPE(&f_swap_2)
        );
    
    const auto x =
        mectx::swap_context(
            ctx
        ,   MEFDN_NONTYPE(&f_swap)
        ,   reinterpret_cast<int*>(111)
        );
    
    ASSERT_EQ(111, reinterpret_cast<uip>(x.p0));
}

