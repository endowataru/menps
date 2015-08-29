
#include <mgcom.hpp>
#include "am_receiver.hpp"
#include "am_sender.hpp"

namespace mgcom {

namespace am {

void initialize()
{
    receiver::initialize();
    sender::initialize();
}

void finalize()
{
    sender::finalize();
    receiver::finalize();
}

}

}

