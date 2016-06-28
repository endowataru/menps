
#pragma once

#include <mgcom/common.h>
#include <mgbase/deferred.hpp>

#include <cstddef>
#include <cstring>

namespace mgcom {

typedef mgcom_index_t                  index_t;

typedef mgcom_process_id_t             process_id_t;


/**
 * Initialize and start the communication.
 */
void initialize(int* argc, char*** argv);

/**
 * Finalize the communication.
 */
void finalize();

class endpoint
    : mgbase::noncopyable
{
public:
    virtual ~endpoint() MGBASE_EMPTY_DEFINITION
    
    process_id_t current_process_id() MGBASE_NOEXCEPT {
        return proc_id_;
    }
    
    index_t number_of_processes() MGBASE_NOEXCEPT {
        return num_procs_;
    }
    
    
    static endpoint& get_instance() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(endpoint_ != MGBASE_NULLPTR);
        return *endpoint_;
    }
    
    static void set_instance(endpoint& ep) MGBASE_NOEXCEPT {
        endpoint_ = &ep;
    }
    
protected:
    endpoint() MGBASE_EMPTY_DEFINITION
    
    void set_current_process_id(const process_id_t proc_id) {
        proc_id_ = proc_id;
    }
    void set_number_of_processes(const index_t num_procs) {
        num_procs_ = num_procs;
    }
    
private:
    process_id_t    proc_id_;
    index_t         num_procs_;
    
    static endpoint* endpoint_;
};

inline process_id_t current_process_id() MGBASE_NOEXCEPT
{
    return endpoint::get_instance().current_process_id();
}

inline index_t number_of_processes() MGBASE_NOEXCEPT
{
    return endpoint::get_instance().number_of_processes();
}

inline bool valid_process_id(process_id_t proc) MGBASE_NOEXCEPT
{
    return proc < number_of_processes();
}

} // namespace mgcom

