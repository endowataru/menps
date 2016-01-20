
#pragma once

#include "receiver.ipp"
#include "sender.ipp"

namespace mgcom {
namespace am {

namespace /*unnamed*/ {

class impl
    : public receiver_impl
    , public sender_impl
{
public:
    void initialize()
    {
        resource_manager::initialize();
        receiver_impl::initialize();
    }
    
    void finalize()
    {
        receiver_impl::finalize();
        resource_manager::finalize();
    }
};

} // unnamed namespace

} // namespace am
} // namespace mgcom

