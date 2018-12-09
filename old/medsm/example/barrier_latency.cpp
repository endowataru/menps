
#include <menps/mecom.hpp>
#include <menps/medsm.hpp>
#include <menps/medsm/segment_ref.hpp>
#include <iostream>
#include <menps/mefdn/profiling/stopwatch.hpp>
#include <menps/mefdn/profiling/time.hpp>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/external/fmt.hpp>

namespace medsm = menps::medsm;
namespace mecom = menps::mecom;
namespace mefdn = menps::mefdn;

int g_dummy = 0;

//ts_t cur_time() {
double cur_time() {
    #ifdef __APPLE__
    return menps::mefdn::get_current_sec();
    #else
    struct timespec ts[1];
    clock_gettime(CLOCK_REALTIME, ts);
    return ts->tv_sec + ts->tv_nsec * 1e-9;
    #endif
}

int main(int argc, char* argv[])
{
    mecom::initialize(&argc, &argv);
    
    {
        auto sp = medsm::make_space();
        
        sp.enable_on_this_thread();
        
        medsm::segment_ref seg;
        
        const int num_changed = 30000;
        const int num_samples = 1000;
        
        mefdn::uint8_t* pi = nullptr;
        if (mecom::current_process_id() == 0) {
            //seg = sp.make_segment(1ull << 20, 1<<15, 1<<12);
            seg = sp.make_segment(1ull << 20, 1<<12, 1<<12);
            pi = static_cast<mefdn::uint8_t*>(seg.get_ptr());
            
            for (int i = 0; i < num_changed; ++i) {
                pi[i] = 0;
            }
        }
        
        mecom::collective::broadcast(0, &pi, 1);
        
        const auto data_read = mefdn::make_unique<mefdn::average_accumulator<double> []>(num_samples);
        const auto data_write = mefdn::make_unique<mefdn::average_accumulator<double> []>(num_samples);
        const auto data_write_barrier = mefdn::make_unique<mefdn::average_accumulator<double> []>(num_samples);
        const auto data_read_barrier = mefdn::make_unique<mefdn::average_accumulator<double> []>(num_samples);
        
        /*mefdn::cpu_clock_t write_cycles[num_changes] = {0};
        mefdn::cpu_clock_t barrier_cycles[num_changes] = {0};*/
        
        const int num_trials = 16;
        
        int dummy = 0;
        
        if (mecom::current_process_id() == 1)
        {
            for (int j = 0; j <= num_trials; j++) {
                for (int i = 0; i < num_samples; ++i) {
                    /*mefdn::stopwatch sw;
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
        
        mecom::collective::barrier();
        
        sp.read_barrier();
        
        using fmt::print;
        
        //for (mecom::process_id_t proc = 0; proc < mecom::number_of_processes(); ++proc)
        if (mecom::current_process_id() == 1)
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
            //std::cout << mecom::current_process_id() << " " << pi[proc] << std::endl;
        }
        
        //*static_cast<int*>(p) = 0;
    }
    
    mecom::finalize();
    
    return 0;
}
