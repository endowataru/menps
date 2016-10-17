
#pragma once

#include <stdexcept>

namespace mgth {

class disable_aslr_error
    : public std::exception {};

void disable_aslr(int argc, char** argv);

} // namespace mgth

