#include "session.h"

namespace scriptor
{

namespace
{

const std::string start = "<c>";
const std::string end = "</m>";

} // namespace

std::vector<Element>
Processor::operator()(const char* const data, const std::size_t length)
{
    m_current.append(data, length);

    auto find_one = [this]() -> std::optional<Element> {
        const auto is = m_current.find(start);
        if (is != std::string::npos)
        {
            const auto ie = m_current.find(end, is);
            if (ie != std::string::npos)
            {
                const auto xml = m_current.substr(is, ie - is + end.size());
                m_current = m_current.substr(ie + end.size());
                try
                {
                    return Element::from_xml(xml);
                }
                catch (...)
                {
                    // invalid xml - ignore.
                }
            }
        }
        return std::nullopt;
    };

    std::vector<Element> elements;
    while (auto&& e = find_one())
    {
        elements.push_back(std::move(*e));
    }
    return elements;
}

Session::Session(aio::local::stream_protocol::socket&& socket,
                 std::function<void(std::vector<Element>&&)> push)
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
                auto&& elements =
                    self->m_processor(self->m_buffer.data(), length);
                if (!elements.empty())
                {
                    self->m_push(std::move(elements));
                }
                self->read();
            }
        });
}

} // namespace scriptor
