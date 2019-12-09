
#pragma once

#include <cmpth/fdn.hpp>
#include <abt.h>

namespace cmpth {

struct abt_error : std::exception {
    static void check_error(const int err_code) {
        if (CMPTH_UNLIKELY(err_code != 0)) {
            throw abt_error();
        }
    }
};

} // namespace cmpth

