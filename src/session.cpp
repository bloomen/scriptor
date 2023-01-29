#include "session.h"
#include <iostream>
namespace scriptor
{

Session::Session(aio::local::stream_protocol::socket&& socket)
    : m_socket{std::move(socket)}
{
}

void
Session::read()
{
    auto self = shared_from_this();
    m_socket.async_read_some(
        aio::buffer(m_buffer),
        [self](const boost::system::error_code ec, const std::size_t length) {
            if (!ec)
            {
                std::cout << "Received:";

                std::cout.write(self->m_buffer.data(), length);
                std::cout << std::endl;
                self->read();
            }
        });
}

} // namespace scriptor
