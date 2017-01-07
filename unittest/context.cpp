
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

#if 0

namespace /*unnamed*/ {

mgctx::context<int*> g_make_main;
mgctx::context<int*> g_make_new;

void f_make_restore(mgctx::transfer<int*> tr)
{
    
}

void f_make_new(mgctx::transfer<int*> tr)
{
    mgctx::restore_context(
        g_make_main
    ,   
    ,   tr
    );
}

} // unnamed namespace


TEST(Context, Make)
{
    const mgbase::size_t stack_size = 4096;
    
    mgbase::unique_ptr<char []> st1{ new char[stack_size] };
    mgbase::unique_ptr<char []> st2{ new char[stack_size] };
    
    g_make_new = mgctx::make_context(
        st1.get() + stack_size
    ,   stack_size
    ,   MGBASE_NONTYPE(&f_make_new)
    );
    
    mgctx::restore_context(
        st1.get() + stack_size
    ,   stack_size
    ,   MGBASE_NONTYPE(&f_make_save)
    ,   reinterpret_cast<int*>(111)
    );
}

#endif

#if 0


mgctx::transfer<int*> f_make_save(mgctx::context<int*> ctx, mgctx::transfer<int*> tr)
{
    g_make_main = ctx;
    
    mgctx::restore_context(
        g_make_new
    ,   
    ,   tr
    );
}
namespace /*unnamed*/ {

mgctx::context<int*> g_make_main;
mgctx::context<int*> g_make_new;

mgctx::transfer<int*>void f_make(mgctx::transfer<int*> tr)
{
    mgctx::restore_context(
        g_ctx_make
    ,   tr.p0 + 1
    );
}

MGBASE_NORETURN
mgctx::transfer<int*> f_make_save(mgctx::context<int*> ctx, int* d)
{
    g_ctx_make = ctx;
    
    mgctx::restore_context(g_make_new);
}

} // namespace unnamed

TEST(Context, Make)
{
    const mgbase::size_t stack_size = 4096;
    
    mgbase::unique_ptr<char []> st1{ new char[stack_size] };
    
    g_make_new = mgctx::make_context(
        st1.get() + stack_size
    ,   stack_size
    ,   MGBASE_NONTYPE(&f_make)
    ,   reinterpret_cast<int*>(111)
    );
    
    const auto x =
        mgctx::save_context(
            st1.get() + stack_size
        ,   stack_size
        ,   MGBASE_NONTYPE(&f_make_save)
        ,   reinterpret_cast<int*>(111)
        );
    
    ASSERT_EQ(111, reinterpret_cast<uip>(x.p0));
}

#endif

