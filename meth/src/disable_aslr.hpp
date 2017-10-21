
#pragma once

#include <menps/meth/common.hpp>
#include <stdexcept>

namespace menps {
namespace meth {

class disable_aslr_error
    : public std::exception {};

void disable_aslr(int argc, char** argv);

} // namespace meth
} // namespace menps

