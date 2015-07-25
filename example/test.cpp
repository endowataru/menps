
#include <mgcom.hpp>
#include <mgbase/stdint.hpp>

int main(int argc, char* argv[])
{
    using namespace mgcom;
    
    initialize(&argc, &argv);
    
    /*mgbase::int32_t val;
    local_region_id_t local_region_id;
    local_addr_t local_addr;
    register_local_region(&val, sizeof(val), &local_region_id, &local_addr);*/
    
    
    
    
    finalize();
    
    return 0;
}

