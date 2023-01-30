#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "aio.h"
#include "element.h"

namespace scriptor
{

class Processor
{
public:
    Processor() = default;

    Processor(const Processor&) = delete;
    Processor&
    operator=(const Processor&) = delete;
    Processor(Processor&&) = delete;
    Processor&
    operator=(Processor&&) = delete;

    std::vector<Element>
    operator()(const char* data, std::size_t length);

private:
    std::string m_current;
};

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(aio::local::stream_protocol::socket&& socket,
            std::function<void(Element&&)> push);

    Session(const Session&) = delete;
    Session&
    operator=(const Session&) = delete;
    Session(Session&&) = delete;
    Session&
    operator=(Session&&) = delete;

    void
    read();

private:
    aio::local::stream_protocol::socket m_socket;
    std::array<char, 1024> m_buffer;
    std::function<void(Element&&)> m_push;
    Processor m_processor;
};

} // namespace scriptor
