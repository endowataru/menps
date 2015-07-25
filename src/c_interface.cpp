
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

mgcom_error_t mgcom_try_put_async(
    mgcom_local_region_id_t  local_region_id
,   mgcom_local_addr_t       local_addr
,   mgcom_remote_region_id_t remote_region_id
,   mgcom_remote_addr_t      remote_addr
,   mgcom_index_t            size_in_bytes
,   mgcom_process_id_t       dest_proc
,   mgcom_notifier_t         on_complete
,   bool*                    succeeded
) MGBASE_NOEXCEPT
{
    CATCH_ERROR(*succeeded =
        mgcom::try_put_async(local_region_id, local_addr, remote_region_id, remote_addr, size_in_bytes, dest_proc, on_complete))
}

}

