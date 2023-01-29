#pragma once

#if __clang__ || __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <boost/asio.hpp>
#if __clang__ || __GNUC__
#pragma GCC diagnostic pop
#endif

namespace aio = boost::asio;
