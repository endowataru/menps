
#include <menps/mecom.hpp>

namespace menps {
namespace mecom {

extern "C" {

#define CATCH_ERROR(statement) \
    try { \
        statement; \
        return MECOM_SUCCESS; \
    } catch (...) { \
        return MECOM_FAILURE; \
    }

mecom_error_code_t mecom_initialize(int* argc, char*** argv) noexcept {
    CATCH_ERROR(mecom::initialize(argc, argv))
}

mecom_error_code_t mecom_rma_remote_write_nb(
    mecom_rma_remote_write_cb*  cb
,   mecom_process_id_t          proc
,   mecom_rma_remote_address    remote_addr
,   mecom_rma_local_address     local_addr
,   mecom_index_t               size_in_bytes
) noexcept
{
    CATCH_ERROR(mecom::rma::untyped::remote_write_nb(*cb, proc, remote_addr, local_addr, size_in_bytes))
}

}

}

