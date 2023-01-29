#include "session.h"

namespace scriptor
{

namespace
{

const std::string start = "<c>";
const std::string end = "</m>";

} // namespace

Session::Session(aio::local::stream_protocol::socket&& socket,
                 std::function<void(Element&&)> push)
    : m_socket{std::move(socket)}
    , m_push{std::move(push)}
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
                self->m_current.append(self->m_buffer.data(), length);
                const auto is = self->m_current.find(start);
                if (is != std::string::npos)
                {
                    const auto ie = self->m_current.find(end);
                    if (ie != std::string::npos)
                    {
                        const auto xml =
                            self->m_current.substr(is, ie + end.size());
                        try
                        {
                            auto element = Element::from_xml(xml);
                            self->m_push(std::move(element));
                        }
                        catch (...)
                        {
                            // invalid xml - ignore.
                        }
                        self->m_current =
                            self->m_current.substr(ie + end.size());
                    }
                }
                self->read();
            }
        });
}

} // namespace scriptor
