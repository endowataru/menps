
#pragma once

#include <menps/mefdn/lang.hpp>
#include <stdexcept>

namespace menps {
namespace mefdn {

class disable_aslr_error
    : public std::exception {};

void disable_aslr(int argc, char** argv);

} // namespace mefdn
} // namespace menps

