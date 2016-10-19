
#include <mgult/sm.hpp>

#include <iostream>

mgult::scheduler_ptr g_s;

void hoge(void*)
{
    std::cout << "hoge" << std::endl;
}

void f()
{
    std::cout << "f()" << std::endl;
    
    auto t = g_s->allocate(0, 0);
    
    g_s->fork(t, hoge);
    
    std::cout << "f().parent " << std::endl;
    
    g_s->join(t.id);
    
}

int main()
{
    g_s = mgult::make_sm_scheduler();
    
    g_s->loop(f);
    
    g_s.reset();
    
    //s->fork(&hoge, MGBASE_NULLPTR);
}

