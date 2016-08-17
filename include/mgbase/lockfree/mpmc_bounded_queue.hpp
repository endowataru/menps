
#pragma once

#error "No longer maintained"

#include <mgbase/lang.hpp>
#include <mgbase/atomic.hpp>
#include <cassert>

namespace mgbase {

// Original implementation is referred from:
// http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

template<typename T>
class mpmc_bounded_queue
    : noncopyable
{
public:
    mpmc_bounded_queue(std::size_t buffer_size)
        : buffer_(new cell_t[buffer_size])
        , buffer_mask_(buffer_size - 1)
    {
        assert((buffer_size >= 2) &&
                ((buffer_size & (buffer_size - 1)) == 0));
        
        for (std::size_t i = 0; i < buffer_size; ++i)
            buffer_[i].sequence_.store(i, memory_order_relaxed);
        
        enqueue_pos_.store(0, memory_order_relaxed);
        dequeue_pos_.store(0, memory_order_relaxed);
    }
    
    ~mpmc_bounded_queue()
    {
        delete[] buffer_;
    }
    
    bool enqueue(const T& data)
    {
        cell_t* cell;
        std::size_t pos = enqueue_pos_.load(mgbase::memory_order_relaxed);
        
        while (true)
        {
            cell = &buffer_[pos & buffer_mask_];
            std::size_t seq = cell->sequence_.load(mgbase::memory_order_acquire);
            
            mgbase::ssize_t diff = static_cast<mgbase::ssize_t>(seq - pos);
            if (diff == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, mgbase::memory_order_relaxed))
                    break;
            }
            else if (diff < 0)
                return false;
            else
                pos = enqueue_pos_.load(mgbase::memory_order_relaxed);
        }
        
        cell->data_ = data;
        cell->sequence_.store(pos + 1, mgbase::memory_order_release);
        
        return true;
    }
    
    bool dequeue(T& data)
    {
        cell_t* cell;
        size_t pos = dequeue_pos_.load(mgbase::memory_order_relaxed);
        
        while (true)
        {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence_.load(mgbase::memory_order_acquire);
            
            mgbase::ssize_t diff = static_cast<mgbase::ssize_t>(seq - (pos + 1));
            if (diff == 0) {
                if (dequeue_pos_.compare_exchange_weak
                        (pos, pos + 1, mgbase::memory_order_relaxed))
                    break;
            }
            else if (diff < 0)
                return false;
            else
                pos = dequeue_pos_.load(mgbase::memory_order_relaxed);
        }
        
        data = cell->data_;
        cell->sequence_.store(pos + buffer_mask_ + 1, mgbase::memory_order_release);
        
        return true;
    }

private:
    struct cell_t
    {
        atomic<std::size_t> sequence_;
        T              data_;
    };
    
    static const std::size_t cacheline_size = 64;
    typedef char cacheline_pad_t[cacheline_size];
    
    cacheline_pad_t     pad0_;
    cell_t* const       buffer_;
    const std::size_t   buffer_mask_;
    cacheline_pad_t     pad1_;
    atomic<std::size_t> enqueue_pos_;
    cacheline_pad_t     pad2_;
    atomic<std::size_t> dequeue_pos_;
    cacheline_pad_t     pad3_;
}; 

}

