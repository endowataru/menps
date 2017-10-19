
#include "example_sm.hpp"
#include "fib.hpp"

int main(const int argc, char** const argv)
{
    menps::meult::sm::initializer init;
    
    example::fib_main(argc, argv);
    
    return 0;
}

