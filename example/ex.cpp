
#include <mgcom.hpp>
#include <mgdsm.hpp>

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    auto sp = mgdsm::make_space();
    
    
    mgcom::finalize();
    
    return 0;
}

