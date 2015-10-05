
#include <mgcom.hpp>

namespace mgcom {

extern "C" {

#define CATCH_ERROR(statement) \
    try { \
        statement; \
        return MGCOM_SUCCESS; \
    } catch (...) { \
        return MGCOM_FAILURE; \
    }

mgcom_error_code_t mgcom_initialize(int* argc, char*** argv) MGBASE_NOEXCEPT {
    CATCH_ERROR(mgcom::initialize(argc, argv))
}

mgcom_error_code_t mgcom_rma_remote_write_async(
    mgcom_rma_remote_write_cb*  cb
,   mgcom_process_id_t          proc
,   mgcom_rma_remote_address    remote_addr
,   mgcom_rma_local_address     local_addr
,   mgcom_index_t               size_in_bytes
) MGBASE_NOEXCEPT
{
    CATCH_ERROR(mgcom::rma::remote_write_nb(*cb, proc, remote_addr, local_addr, size_in_bytes))
}

}

}

