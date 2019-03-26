
#pragma once

#include <string>
#include <sstream>

namespace mgbase {

unsigned long stoul(const std::string& str)
{
    std::istringstream ss(str);
    unsigned long ret;
    ss >> ret;
    return ret;
}

} // namespace mgbase

