
#include <mgcom.hpp>
#include <mgdsm.hpp>

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    {
        auto sp = mgdsm::make_space();
        
        auto seg = sp.make_segment(1ull << 20, 100, 10);
        
        auto p = seg.get_ptr();
        
        sp.enable_on_this_thread();
        
        *static_cast<int*>(p) = 0;
    }
    
    mgcom::finalize();
    
    return 0;
}

