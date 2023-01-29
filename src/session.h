#pragma once

#include <array>
#include <memory>
#include <string>

#include "aio.h"

namespace scriptor
{

class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(aio::local::stream_protocol::socket&& socket);

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
};

} // namespace scriptor
