
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

mgcom_error_t mgcom_try_write_async(
    mgcom_local_address_t          local_address
,   mgcom_remote_address_t         remote_address
,   mgcom_index_t                  size_in_bytes
,   mgcom_process_id_t             dest_proc
,   mgcom_notifier_t               on_complete
,   bool*                          succeeded
) MGBASE_NOEXCEPT
{
    CATCH_ERROR(*succeeded = mgcom::try_write_async(local_address, remote_address, size_in_bytes, dest_proc, on_complete))
}

}

