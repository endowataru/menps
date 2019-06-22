
#include "unittest.hpp"
#include <menps/meult/fcontext.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace mefdn = menps::mefdn;
namespace meult = menps::meult;

namespace /*unnamed*/ {

//fcontext g_jump3_root;
//meult::fcontext<int, char> g_jump3_1;
meult::fcontext<double, short> g_jump3_2;
//fcontext<int, char> g_jump3_3;

void jump3_1(meult::fcontext_argument<int, char> a)
{
    /*char* x = nullptr;
    auto r = meult::jump_fcontext(, x);*/
    
    const auto i1 = reinterpret_cast<mefdn::uintptr_t>(a.data);
    
    const auto f = meult::jump_fcontext<long>(g_jump3_2, reinterpret_cast<short*>(i1 + 1));
    
    const auto i3 = reinterpret_cast<mefdn::uintptr_t>(f.data);
    
    meult::jump_fcontext<void>(a.fctx, reinterpret_cast<int*>(i3 + 100));
    
}
void jump3_2(meult::fcontext_argument<double, short> a)
{
    const auto i2 = reinterpret_cast<mefdn::uintptr_t>(a.data);
    
    meult::jump_fcontext<void>(a.fctx, reinterpret_cast<double*>(i2 + 10));
    
}
/*void jump3_3() {
    
}*/

} // unnamed namespace

TEST(Fcontext, Jump3)
{
    const mefdn::size_t stack_size = 4096;
    
    mefdn::unique_ptr<char []> st1{ new char[stack_size] };
    mefdn::unique_ptr<char []> st2{ new char[stack_size] };
    mefdn::unique_ptr<char []> st3{ new char[stack_size] };
    
    //g_jump3_1 = meult::make_fcontext(st1.get() + stack_size, stack_size, jump3_1);
    g_jump3_2 = meult::make_fcontext(st2.get() + stack_size, stack_size, jump3_2);
    //g_jump3_3 = meult::make_fcontext(st3.get() + stack_size, stack_size, jump3_3);
    
    const auto root = meult::make_fcontext(st1.get() + stack_size, stack_size, jump3_1);
    
    char* nul = nullptr;
    const auto ret = meult::jump_fcontext<void>(root, nul);
    
    ASSERT_EQ(111, reinterpret_cast<mefdn::uintptr_t>(ret.data));
    
}

