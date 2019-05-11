
#pragma once

#include <menps/medev2/mpi.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/external/googletest.hpp>

using namespace menps;


#ifdef MEFDN_ENABLE_MEULT
#include <menps/meult/backend/mth/ult_policy.hpp>
using ult_itf_t = meult::backend::mth::ult_policy; // TODO

#else
#include <cmpth/smth/default_smth.hpp>
using ult_itf_t = cmpth::default_smth_itf;
#endif
using direct_mpi_itf_t = medev2::mpi::direct_mpi_itf<ult_itf_t>;

extern direct_mpi_itf_t::mpi_facade_type* g_mi;

