
#pragma once

#include <cmpth/ult_ext_itf.hpp>
#include <cmpth/sct/def_sct_itf.hpp>

using fibre_itf = cmpth::get_ult_ext_itf_t<cmpth::ult_tag_t::SCT, cmpth::sync_tag_t::MCS>;

/*class FibreMutex
    : public fibre_itf::mutex
{
public:
    void acquire() { this->lock(); }
    void release() { this->unlock(); }
};

class FibreCondition
    : public fibre_itf::condition_variable
{
public:
    void signal() { this->notify(); }
};*/

class Fibre
{
public:
    Fibre(void (*start_routine)(void *), void* arg) {
        this->t_ = fibre_itf::thread::ptr_fork(start_routine, arg);
    }
    ~Fibre() {
        this->t_.join();
    }

private:
    fibre_itf::thread t_;
};


