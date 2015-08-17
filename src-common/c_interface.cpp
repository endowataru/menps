
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
    mgcom_write_cb*      cb
,   mgcom_local_address  local_addr
,   mgcom_remote_address remote_addr
,   mgcom_index_t        size_in_bytes
,   mgcom_process_id_t   dest_proc
) MGBASE_NOEXCEPT
{
    CATCH_ERROR(mgcom::write_async(cb, local_addr, remote_addr, size_in_bytes, dest_proc))
}

}

