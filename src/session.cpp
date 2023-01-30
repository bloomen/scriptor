#include "session.h"

namespace scriptor
{

namespace
{

const std::string start = "<c>";
const std::string end = "</m>";

} // namespace

std::optional<Element>
Processor::operator()(const char* const data, const std::size_t length)
{
    m_current.append(data, length);
    const auto is = m_current.find(start);
    if (is != std::string::npos)
    {
        const auto ie = m_current.find(end, is);
        if (ie != std::string::npos)
        {
            const auto xml = m_current.substr(is, ie - is + end.size());
            try
            {
                return Element::from_xml(xml);
            }
            catch (...)
            {
                // invalid xml - ignore.
            }
            m_current = m_current.substr(ie + end.size());
        }
    }
    return std::nullopt;
}

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
                if (auto element =
                        self->m_processor(self->m_buffer.data(), length))
                {
                    self->m_push(std::move(*element));
                }
                self->read();
            }
        });
}

} // namespace scriptor
