
#pragma once

#include "device/ibv/command/command.hpp"

#ifdef MECOM_IBV_ENABLE_SLEEP_QP
    #include <menps/mefdn/nonblocking/mpsc_locked_bounded_queue.hpp>
#else
    #include <menps/mefdn/nonblocking/mpsc_bounded_queue.hpp>
#endif

#include <menps/mecom/ult.hpp>

namespace menps {
namespace mecom {
namespace ibv {

class command_queue
#ifdef MECOM_IBV_ENABLE_SLEEP_QP
    : public mefdn::static_mpsc_locked_bounded_queue<command, command::queue_size>
{
//protected:
public: // TODO, XXX
    typedef ult::unique_lock<ult::mutex>    unique_lock_type;
    
    void notify()
    {
        const auto lk = this->get_lock();
        cv_.notify_one();
    }
    
    unique_lock_type get_lock() {
        return unique_lock_type(mtx_);
    }
    
    void wait(unique_lock_type& lk) {
        this->cv_.wait(lk);
    }
    
private:
    ult::mutex mtx_;
    ult::condition_variable cv_;
};
#else
    : public mefdn::static_mpsc_bounded_queue<command, command::queue_size> { };
#endif

} // namespace ibv
} // namespace mecom
} // namespace menps

