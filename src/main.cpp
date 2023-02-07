// scriptor - A high-performance logger using unix/tcp sockets
// Repo: https://github.com/bloomen/scriptor
// Author: Christian Blume
// License: MIT http://www.opensource.org/licenses/mit-license.php

#include <csignal>

#include "scriptor.h"

void
signal_handler(int signal)
{
    scriptor::stop(signal);
}

int
main(int argc, char** argv)
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    return scriptor::run(argc, argv);
}
