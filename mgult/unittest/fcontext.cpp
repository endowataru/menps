
#include "unittest.hpp"
#include <mgult/fcontext.hpp>
#include <mgbase/unique_ptr.hpp>

namespace /*unnamed*/ {

//fcontext g_jump3_root;
//mgult::fcontext<int, char> g_jump3_1;
mgult::fcontext<double, short> g_jump3_2;
//fcontext<int, char> g_jump3_3;

void jump3_1(mgult::fcontext_argument<int, char> a)
{
    /*char* x = MGBASE_NULLPTR;
    auto r = mgult::jump_fcontext(, x);*/
    
    const auto i1 = reinterpret_cast<mgbase::uintptr_t>(a.data);
    
    const auto f = mgult::jump_fcontext<long>(g_jump3_2, reinterpret_cast<short*>(i1 + 1));
    
    const auto i3 = reinterpret_cast<mgbase::uintptr_t>(f.data);
    
    mgult::jump_fcontext<void>(a.fctx, reinterpret_cast<int*>(i3 + 100));
    
}
void jump3_2(mgult::fcontext_argument<double, short> a)
{
    const auto i2 = reinterpret_cast<mgbase::uintptr_t>(a.data);
    
    mgult::jump_fcontext<void>(a.fctx, reinterpret_cast<double*>(i2 + 10));
    
}
/*void jump3_3() {
    
}*/

} // unnamed namespace

TEST(Fcontext, Jump3)
{
    const mgbase::size_t stack_size = 4096;
    
    mgbase::unique_ptr<char []> st1{ new char[stack_size] };
    mgbase::unique_ptr<char []> st2{ new char[stack_size] };
    mgbase::unique_ptr<char []> st3{ new char[stack_size] };
    
    //g_jump3_1 = mgult::make_fcontext(st1.get() + stack_size, stack_size, jump3_1);
    g_jump3_2 = mgult::make_fcontext(st2.get() + stack_size, stack_size, jump3_2);
    //g_jump3_3 = mgult::make_fcontext(st3.get() + stack_size, stack_size, jump3_3);
    
    const auto root = mgult::make_fcontext(st1.get() + stack_size, stack_size, jump3_1);
    
    char* nul = MGBASE_NULLPTR;
    const auto ret = mgult::jump_fcontext<void>(root, nul);
    
    ASSERT_EQ(111, reinterpret_cast<mgbase::uintptr_t>(ret.data));
    
}

