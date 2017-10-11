
#include <mgcom.hpp>
#include <mgdsm.hpp>
#include <mgdsm/segment_ref.hpp>
#include <iostream>
#include <mgbase/profiling/stopwatch.hpp>
#include <mgbase/profiling/time.hpp>
#include <mgbase/profiling/average_accumulator.hpp>
#include <mgbase/external/fmt.hpp>

int g_dummy = 0;

//ts_t cur_time() {
double cur_time() {
  struct timespec ts[1];
  clock_gettime(CLOCK_REALTIME, ts);
  return ts->tv_sec + ts->tv_nsec * 1e-9;
}

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    {
        auto sp = mgdsm::make_space();
        
        sp.enable_on_this_thread();
        
        mgdsm::segment_ref seg;
        
        const int num_changed = 30000;
        const int num_samples = 1000;
        
        mgbase::uint8_t* pi = MGBASE_NULLPTR;
        if (mgcom::current_process_id() == 0) {
            //seg = sp.make_segment(1ull << 20, 1<<15, 1<<12);
            seg = sp.make_segment(1ull << 20, 1<<12, 1<<12);
            pi = static_cast<mgbase::uint8_t*>(seg.get_ptr());
            
            for (int i = 0; i < num_changed; ++i) {
                pi[i] = 0;
            }
        }
        
        mgcom::collective::broadcast(0, &pi, 1);
        
        const auto data_read = mgbase::make_unique<mgbase::average_accumulator<double> []>(num_samples);
        const auto data_write = mgbase::make_unique<mgbase::average_accumulator<double> []>(num_samples);
        const auto data_write_barrier = mgbase::make_unique<mgbase::average_accumulator<double> []>(num_samples);
        const auto data_read_barrier = mgbase::make_unique<mgbase::average_accumulator<double> []>(num_samples);
        
        /*mgbase::cpu_clock_t write_cycles[num_changes] = {0};
        mgbase::cpu_clock_t barrier_cycles[num_changes] = {0};*/
        
        const int num_trials = 16;
        
        int dummy = 0;
        
        if (mgcom::current_process_id() == 1)
        {
            for (int j = 0; j <= num_trials; j++) {
                for (int i = 0; i < num_samples; ++i) {
                    /*mgbase::stopwatch sw;
                    sw.start();*/
                    
                    const auto t0 = cur_time();
                    
                    for (int k = 0; k < i*(num_changed/num_samples); ++k) {
                        dummy += pi[k];
                    }
                    
                    const auto t1 = cur_time();
                    
                    for (int k = 0; k < i*(num_changed/num_samples); ++k) {
                        pi[k] = j;
                    }
                    
                    const auto t2 = cur_time();
                    
                    sp.write_barrier();
                    
                    const auto t3 = cur_time();
                    
                    sp.read_barrier();
                    
                    const auto t4 = cur_time();
                    
                    if (j > 0) {
                        data_read[i].add(t1 - t0);
                        data_write[i].add(t2 - t1);
                        data_write_barrier[i].add(t3 - t2);
                        data_read_barrier[i].add(t4 - t3);
                    }
                }
            }
        }
        
        g_dummy = dummy;
        
        mgcom::collective::barrier();
        
        sp.read_barrier();
        
        using fmt::print;
        
        //for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
        if (mgcom::current_process_id() == 1)
        {
            for (int i = 0; i < num_samples; ++i) {
                print("- changed : {} # [bytes]\n", i*(num_changed/num_samples));
                print("  read_time : {}\n", data_read[i].summary());
                print("  write_time : {}\n", data_write[i].summary());
                print("  data_write_barrier: {}\n", data_write_barrier[i].summary());
                print("  data_read_barrier: {}\n", data_read_barrier[i].summary());
                /*std::cout << i << ", write = " << (1.0*write_cycles[i]/num_trials) << //<< std::endl;
                    ", write barrier = " << (1.0*barrier_cycles[i]/num_trials) << std::endl;*/
            }
            //std::cout << mgcom::current_process_id() << " " << pi[proc] << std::endl;
        }
        
        //*static_cast<int*>(p) = 0;
    }
    
    mgcom::finalize();
    
    return 0;
}
