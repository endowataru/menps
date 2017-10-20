
#pragma once

#include <menps/mecom/common.h>
#include <menps/mefdn/assert.hpp>

#include <cstddef>
#include <cstring>

namespace menps {
namespace mecom {

namespace rma {

class requester;

class registrator;

class allocator;

} // namespace rma

namespace rpc {

class requester;

} // namespace rpc

namespace collective {

class requester;

} // namespace collective



typedef mecom_index_t                  index_t;

typedef mecom_process_id_t             process_id_t;


/**
 * Initialize and start the communication.
 */
void initialize(int* argc, char*** argv);

/**
 * Finalize the communication.
 */
void finalize();

class endpoint
{
public:
    virtual ~endpoint() /*noexcept*/ = default;
    
    endpoint(const endpoint&) = delete;
    endpoint& operator = (const endpoint&) = delete;
    
    process_id_t current_process_id() noexcept {
        return proc_id_;
    }
    
    index_t number_of_processes() noexcept {
        return num_procs_;
    }
    
    
    static endpoint& get_instance() noexcept {
        MEFDN_ASSERT(endpoint_ != nullptr);
        return *endpoint_;
    }
    
    static void set_instance(endpoint& ep) noexcept {
        endpoint_ = &ep;
    }
    
protected:
    endpoint() noexcept = default;
    
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

inline process_id_t current_process_id() noexcept
{
    return endpoint::get_instance().current_process_id();
}

inline index_t number_of_processes() noexcept
{
    return endpoint::get_instance().number_of_processes();
}

inline bool valid_process_id(endpoint& ep, process_id_t proc) noexcept
{
    return proc < ep.number_of_processes();
}
inline bool valid_process_id(process_id_t proc) noexcept
{
    return proc < number_of_processes();
}

} // namespace mecom
} // namespace menps

