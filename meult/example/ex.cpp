
#include <menps/meult/sm.hpp>

#include <iostream>

namespace meult = menps::meult;

namespace /*unnamed*/ {

void hoge(void*)
{
    std::cout << "hoge" << std::endl;
}

void f()
{
    std::cout << "f()" << std::endl;
    
    auto& s = meult::sm::get_scheduler();
    
    auto t = s.allocate(0, 0);
    
    s.fork(t, hoge);
    
    std::cout << "f().parent " << std::endl;
    
    s.join(t.id);
}

} // unnamed namespace

int main()
{
    meult::sm::initializer init;
    
    f();
    
    return 0;
}

