
#pragma once

#include "command_producer.hpp"
#include "command_consumer.hpp"

namespace menps {
namespace mecom {
namespace ibv {

class serializer
    : public command_producer
    , public command_consumer
{
public:
    using command_consumer::config;
    
    explicit serializer(const config& conf)
        : command_consumer(conf) { }
};

} // namespace ibv
} // namespace mecom
} // namespace menps

