#include "session.h"

#include <algorithm>
#include <optional>

namespace scriptor
{

namespace
{

constexpr char start = '{';
constexpr char end = '}';

} // namespace

std::vector<Element>
Processor::operator()(const char* const data, const std::size_t length)
{
    m_current.insert(m_current.end(), data, data + length);

    auto find_one = [this]() -> std::optional<Element> {
        const auto is = std::find(m_current.begin(), m_current.end(), start);
        if (is != m_current.end())
        {
            const auto ie = std::find(is + 1, m_current.end(), end);
            if (ie != m_current.end())
            {
                const auto beyond = ie + 1;
                try
                {
                    auto element = Element::from_json(&*is, &*ie + 1);
                    m_current.erase(m_current.begin(), beyond);
                    return element;
                }
                catch (std::exception& e)
                {
                    m_current.erase(m_current.begin(), beyond);
                    return Element{
                        spdlog::log_clock::now(),
                        spdlog::level::critical,
                        fmt::format("Scriptor Json Parse Error: {}", e.what())};
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

Session::Session(std::unique_ptr<Socket>&& socket,
                 std::function<void(std::vector<Element>&&)>&& push)
    : m_socket{std::move(socket)}
    , m_push{std::move(push)}
{
}

void
Session::read()
{
    m_socket->async_read_some(
        asio::buffer(m_buffer),
        [self = shared_from_this()](const auto ec, const std::size_t length) {
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
