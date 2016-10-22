
#include <mgult/sm.hpp>

#include <iostream>

namespace /*unnamed*/ {

void hoge(void*)
{
    std::cout << "hoge" << std::endl;
}

void f()
{
    std::cout << "f()" << std::endl;
    
    auto& s = mgult::sm::get_scheduler();
    
    auto t = s.allocate(0, 0);
    
    s.fork(t, hoge);
    
    std::cout << "f().parent " << std::endl;
    
    s.join(t.id);
}

} // unnamed namespace

int main()
{
    mgult::sm::initializer init;
    
    f();
    
    return 0;
}

