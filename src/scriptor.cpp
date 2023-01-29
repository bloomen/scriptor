// scriptor - A high-performance logger for Linux using unix file sockets
// Repo: https://github.com/bloomen/scriptor
// Author: Christian Blume
// License: MIT http://www.opensource.org/licenses/mit-license.php

#include "server.h"

#include <atomic>
#include <csignal>
#include <filesystem>
#include <iostream>

using namespace scriptor;

std::atomic<int> g_signal{0};

void
signal_handler(int signal)
{
    g_signal = signal;
}

int
main(int argc, char** argv)
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    try
    {
        if (argc < 2)
        {
            // TODO use boost prog args
            throw std::runtime_error{
                "Socket file not given. Usage: scriptor <socket_file>"};
        }
        const std::filesystem::path path{argv[1]};
        if (std::filesystem::exists(path))
        {
            std::filesystem::remove(path);
        }
        Server server{std::thread::hardware_concurrency(), path.u8string()};
        while (g_signal == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Fatal Error: Unknown" << std::endl;
        return 1;
    }
    return 0;
}
