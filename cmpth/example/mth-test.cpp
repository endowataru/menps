
#include <cmpth/wrap/mth_itf.hpp>
#include <iostream>

void f()
{
    std::cout << "f()" << std::endl;
}

int main()
{
    cmpth::mth_itf::thread t(f);
    t.join();
    t.detach();
    
    return 0;
}

