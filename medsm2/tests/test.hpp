
#pragma once

#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

TEST_CASE("Read an integer on heap") {
    int* const ptr = dsm_itf::conew<int>();
    dsm_itf::spmd_single([ptr] {
        *ptr = 123;
    });
    REQUIRE(*ptr == 123);
    dsm_itf::spmd_parallel([ptr] {
        REQUIRE(*ptr == 123);
    });
    dsm_itf::codelete(ptr);
}

TEST_CASE("Write all thread IDs on array") {
    const auto num_ths = dsm_itf::max_num_threads();
    int* const ptr = dsm_itf::conew_array<int>(num_ths);
    dsm_itf::spmd_parallel([ptr] {
        const auto th_num = dsm_itf::get_thread_num();
        ptr[th_num] = th_num + 100;
    });
    for (int i = 0; i < num_ths; ++i) {
        REQUIRE(ptr[i] == i+100);
    }
    dsm_itf::codelete_array(ptr);
}

