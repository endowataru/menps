
#include <mgult/sm.hpp>

#include <iostream>

mgult::scheduler_ptr g_s;

void* hoge(void*)
{
    std::cout << "hoge" << std::endl;
    return reinterpret_cast<void*>(1);
}

void f()
{
    std::cout << "f()" << std::endl;
    
    auto t = g_s->fork(hoge, MGBASE_NULLPTR);
    
    std::cout << "f().parent " << std::endl;
    
    auto x = g_s->join(t);
    
    std::cout << reinterpret_cast<mgbase::uintptr_t>(x) << std::endl;
    
}

int main()
{
    g_s = mgult::make_sm_scheduler();
    
    g_s->loop(f);
    
    g_s.reset();
    
    //s->fork(&hoge, MGBASE_NULLPTR);
}

