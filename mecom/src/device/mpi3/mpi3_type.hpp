
#pragma once

#include "mpi3_error.hpp"

namespace menps {
namespace mecom {
namespace mpi3 {

inline std::string get_datatype_name(const MPI_Datatype datatype)
{
    char buf[MPI_MAX_OBJECT_NAME];
    int len;
    
    mpi3_error::check(
        MPI_Type_get_name(datatype, buf, &len)
    );
    
    return buf;
}

namespace detail {

template <typename T>
struct mpi_type_base;

template <>
struct mpi_type_base<mefdn::uint64_t>
{
    static MPI_Datatype datatype() noexcept { return MPI_UINT64_T; }
};

} // namespace detail

// TODO: separate to a different header

template <typename T>
struct mpi_type
    : detail::mpi_type_base<T>
{
    typedef T   type;
    
    static std::string name() { return get_datatype_name(mpi_type::datatype()); }
};

} // namespace mpi3
} // namespace mecom
} // namespace menps

