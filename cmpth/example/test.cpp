
#include <cmpth/smth/default_smth.hpp>
#include <iostream>

void my_yield()
{
    cmpth::default_smth_itf::this_thread::yield();
}

void func(char x, int l) {
    for (int i = 0; i < l; ++i) {
        std::cout << x;
        cmpth::default_smth_itf::this_thread::yield();
    }
    //std::cout << std::endl;
}

void w_main()
{
    //std::cout << "w_main";
    cmpth::default_smth_itf::thread t(func, 'A', 120);
    func('B', 100);
    t.join();
    
    /*cmpth::default_smth_itf::uncond_var u;
    u.notify();*/
    
    cmpth::default_smth_itf::mutex mtx;
    mtx.lock();
    mtx.unlock();
    
    //cmpth::default_smth_itf::cond_var cv;
    //cmpth::fdn::unique_lock<decltype(mtx)> lk(mtx);
    //cv.wait(lk);
    //cv.notify_one();
}

int main()
{
    cmpth::smth_scheduler<cmpth::default_smth_policy> sched;
    
    //sched.make_workers(2);
    sched.make_workers(1);
    
    
    //sched.get_worker_of_num(0).local_push_top();
    
    //sched.loop_workers();
    
    #if 1
    sched.execute_workers(w_main);
    #else
    
    {
    cmpth::smth_scheduler<cmpth::default_smth_policy>::initializer init{sched};
    w_main();
    }
    #endif
    
    
    
    #if 0
    /*wk.yield();*/
    
    cmpth::default_smth_itf::thread t(func, 1);
    
    t.join();
    #endif
    
    return 0;
}

