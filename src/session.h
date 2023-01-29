#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>

#include "aio.h"
#include "element.h"

namespace scriptor
{

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
    std::string m_current;
};

} // namespace scriptor
