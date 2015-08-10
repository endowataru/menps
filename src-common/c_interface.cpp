
#include <mgcom.hpp>

extern "C" {

#define CATCH_ERROR(statement) \
    try { \
        statement; \
        return MGCOM_SUCCESS; \
    } catch (...) { \
        return MGCOM_FAILURE; \
    }

mgcom_error_t mgcom_initialize(int* argc, char*** argv) MGBASE_NOEXCEPT {
    CATCH_ERROR(mgcom::initialize(argc, argv))
}

mgcom_error_t mgcom_write_async(
    mgbase_async_request*          request
,   mgcom_local_address_t          local_address
,   mgcom_remote_address_t         remote_address
,   mgcom_index_t                  size_in_bytes
,   mgcom_process_id_t             dest_proc
) MGBASE_NOEXCEPT
{
    CATCH_ERROR(mgcom::write_async(request, local_address, remote_address, size_in_bytes, dest_proc))
}

}

