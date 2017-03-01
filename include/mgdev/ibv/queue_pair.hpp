
#pragma once

#include <mgdev/ibv/verbs.hpp>
#include <mgbase/unique_ptr.hpp>
#include <mgdev/ibv/ibv_error.hpp>
#include <mgbase/assert.hpp>

namespace mgdev {
namespace ibv {

#ifdef MGDEV_IBV_EXP_SUPPORTED
    typedef ibv_exp_qp_init_attr    qp_init_attr_t;
    typedef ibv_exp_qp_attr         qp_attr_t;
#else
    typedef ibv_qp_init_attr        qp_init_attr_t;
    typedef ibv_qp_attr             qp_attr_t;
#endif

qp_init_attr_t make_default_rc_qp_init_attr();

qp_attr_t make_default_qp_attr();
qp_attr_t make_default_qp_attr(const ibv_device_attr&);

void set_qp_dest(qp_attr_t* attr, global_qp_id dest_qp_id);

void modify_reset_to_init(ibv_qp*, qp_attr_t*);
void modify_init_to_rtr(ibv_qp*, qp_attr_t*);
void modify_rtr_to_rts(ibv_qp*, qp_attr_t*);


struct queue_pair_deleter
{
    void operator () (ibv_qp*) const MGBASE_NOEXCEPT;
};

class queue_pair
    : public mgbase::unique_ptr<ibv_qp, queue_pair_deleter>
{
    typedef mgbase::unique_ptr<ibv_qp, queue_pair_deleter>  base;
    
public:
    queue_pair() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit queue_pair(ibv_qp* const qp)
        : base(qp)
    { }
    
    ~queue_pair() /*noexcept*/ = default;
    
    queue_pair(const queue_pair&) = delete;
    queue_pair& operator = (const queue_pair&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(queue_pair, base)
    
    
    mgbase::uint32_t get_qp_num() const MGBASE_NOEXCEPT {
        return (*this)->qp_num;
    }
    
    void init(const port_num_t port_num, qp_attr_t* const attr) const {
        attr->port_num = port_num;
        modify_reset_to_init(this->get(), attr);
    }
    void init(const port_num_t port_num) const {
        auto attr = make_default_qp_attr();
        this->init(port_num, &attr);
    }
    
    void connect_to(const global_qp_id dest_qp_id, qp_attr_t* const attr) const {
        set_qp_dest(attr, dest_qp_id);
        
        modify_init_to_rtr(this->get(), attr);
        modify_rtr_to_rts(this->get(), attr);
    }
    void connect_to(const global_qp_id dest_qp_id) const {
        auto attr = make_default_qp_attr();
        this->connect_to(dest_qp_id, &attr);
    }
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_post_send(ibv_send_wr& wr, ibv_send_wr** const bad_wr) const
    {
        MGBASE_ASSERT(this->get() != MGBASE_NULLPTR);
        
        const int err = ibv_post_send(this->get(), &wr, bad_wr);
        
        if (MGBASE_LIKELY(err == 0)) {
            log_wr("Posted IBV request.", wr);
            return true;
        }
        else if (err == ENOMEM) {
            log_wr("Used up the memory for posting IBV request.", wr);
            return false;
        }
        else {
            log_wr("Failed to post an IBV request.", wr);
            throw ibv_error("ibv_post_send() failed", err);
        }
    }
    
private:
    #ifdef MGBASE_DEBUG
    void log_wr(const char* const msg, const ibv_send_wr& wr) const {
        log_wr_impl(msg, wr);
    }
    #else
    void log_wr(const char* const /*msg*/, const ibv_send_wr& /*wr*/) const { }
    #endif
    
    void log_wr_impl(const char* const msg, const ibv_send_wr& wr) const;
};

queue_pair make_queue_pair(ibv_pd* pd, qp_init_attr_t* attr);


} // namespace ibv
} // namespace mgdev

