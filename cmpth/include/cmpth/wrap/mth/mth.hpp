
#pragma once

#include <cmpth/fdn.hpp>
#include <myth/myth.h>

namespace cmpth {

struct mth_error : std::exception {
    static void check_error(const int err_code) {
        if (CMPTH_UNLIKELY(err_code != 0)) {
            throw mth_error();
        }
    }
};

} // namespace cmpth

