
#include <mgcom.hpp>
#include "am/receiver.hpp"
#include "am/sender.hpp"

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

