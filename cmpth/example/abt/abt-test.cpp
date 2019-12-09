
#include <cmpth/wrap/abt/abt_itf.hpp>
#include <iostream>

void f() {
    std::cout << "a";
}

int main(int argc, char** argv)
{
    cmpth::abt_itf::initializer sched_init{argc, argv};

    cmpth::abt_itf::thread t{f};
    t.join();
}

