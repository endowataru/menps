
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/medev2/mpi.hpp>
#include <menps/meqdc/mpi.hpp>

namespace menps {
namespace mecom2 {

enum class mpi_id_t {
    direct = 1
,   qdc
};

template <mpi_id_t Id>
struct get_mpi_itf_type;

template <>
struct get_mpi_itf_type<mpi_id_t::direct>
    : mefdn::type_identity<medev2::mpi::default_direct_mpi_itf> { };

template <>
struct get_mpi_itf_type<mpi_id_t::qdc>
    : mefdn::type_identity<
        meqdc::proxy_mpi_itf<medev2::mpi::default_direct_mpi_itf>
    > { };

template <mpi_id_t Id>
using get_mpi_itf_type_t = typename get_mpi_itf_type<Id>::type;

#if 0
template <typename P>
struct basic_mpi_com_policy
{
    using mpi_itf_type = MpiItf;
    using ult_itf_type = typename mpi_itf_type::ult_itf_type;
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    template <typename T>
    static MPI_Datatype get_mpi_datatype() {
        return medev2::mpi::get_datatype<T>()();
    }
    
};
#endif

} // namespace mecom2
} // namespace menps

