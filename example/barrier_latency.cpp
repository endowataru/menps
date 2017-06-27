
#include <mgcom.hpp>
#include <mgdsm.hpp>
#include <mgdsm/segment_ref.hpp>
#include <iostream>
#include <mgbase/profiling/stopwatch.hpp>

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    {
        auto sp = mgdsm::make_space();
        
        sp.enable_on_this_thread();
        
        mgdsm::segment_ref seg;
        
        const int num_changes = 10000;
        
        int* pi = MGBASE_NULLPTR;
        if (mgcom::current_process_id() == 0) {
            //seg = sp.make_segment(1ull << 20, 1<<15, 1<<12);
            seg = sp.make_segment(1ull << 20, 1<<12, 1<<12);
            pi = static_cast<int*>(seg.get_ptr());
            
            for (int i = 0; i < num_changes; ++i) {
                pi[i] = 0;
            }
        }
        
        mgcom::collective::broadcast(0, &pi, 1);
        
        mgbase::cpu_clock_t write_cycles[num_changes] = {0};
        mgbase::cpu_clock_t barrier_cycles[num_changes] = {0};
        
        const int num_trials = 10;
        
        if (mgcom::current_process_id() == 1)
        {
            for (int i = 0; i < num_changes; ++i) {
                for (int j = 0; j < num_trials; j++) {
                    mgbase::stopwatch sw;
                    sw.start();
                    
                    for (int k = 0; k < i; ++k) {
                        pi[k] = ++j;
                    }
                    
                    write_cycles[i] += sw.elapsed();
                    sw.start();
                    
                    sp.write_barrier();
                    
                    barrier_cycles[i] += sw.elapsed();
                    
                    sp.read_barrier();
                }
            }
        }
        
        mgcom::collective::barrier();
        
        sp.read_barrier();
        
        //for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
        if (mgcom::current_process_id() == 1)
        {
            for (int i = 0; i < num_changes; ++i) {
                std::cout << i << ", write = " << (1.0*write_cycles[i]/num_trials) << //<< std::endl;
                    ", write barrier = " << (1.0*barrier_cycles[i]/num_trials) << std::endl;
            }
            //std::cout << mgcom::current_process_id() << " " << pi[proc] << std::endl;
        }
        
        //*static_cast<int*>(p) = 0;
    }
    
    mgcom::finalize();
    
    return 0;
}
