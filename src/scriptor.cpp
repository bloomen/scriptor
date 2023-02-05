// scriptor - A high-performance logger using unix/tcp sockets
// Repo: https://github.com/bloomen/scriptor
// Author: Christian Blume
// License: MIT http://www.opensource.org/licenses/mit-license.php

#include <atomic>
#include <csignal>
#include <filesystem>
#include <iostream>

#include "popl.hpp"

#include "server.h"

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
    popl::OptionParser op{
        "scriptor - A high-performance logger using unix/tcp sockets"};
    auto help = op.add<popl::Switch>("h", "help", "Display this help message");
    try
    {
        Options opt;

        op.add<popl::Value<std::string>, popl::Attribute::required>(
            "",
            "socket_file",
            "The unix socket filename (required)",
            "",
            &opt.socket_file);
        op.add<popl::Value<std::string>>(
            "", "identity", "The identity name", opt.identity, &opt.identity);

        // file logging
        op.add<popl::Value<std::string>>("",
                                         "filelog_filename",
                                         "The filelog filename",
                                         std::string{},
                                         &opt.file_sink_filename);
        op.add<popl::Value<std::size_t>>("",
                                         "filelog_max_file_size",
                                         "The filelog max file size",
                                         opt.file_sink_max_file_size,
                                         &opt.file_sink_max_file_size);
        op.add<popl::Value<std::size_t>>("",
                                         "file_sink_max_files",
                                         "The filelog max files",
                                         opt.file_sink_max_files,
                                         &opt.file_sink_max_files);
        op.add<popl::Value<int>>("",
                                 "filelog_level",
                                 "The filelog log level",
                                 opt.file_sink_log_level,
                                 &opt.file_sink_log_level);

#ifdef SCRIPTOR_LINUX
        // systemd logging
        op.add<popl::Switch>("",
                             "systemd_logging",
                             "Enables logging to systemd (Linux only)",
                             &opt.systemd_sink_use);
        op.add<popl::Value<int>>("",
                                 "systemd_level",
                                 "The systemd log level",
                                 opt.systemd_sink_log_level,
                                 &opt.systemd_sink_log_level);

        // syslog logging
        op.add<popl::Switch>("",
                             "syslog_logging",
                             "Enables logging to syslog (Linux only)",
                             &opt.syslog_sink_use);
        op.add<popl::Value<int>>("",
                                 "syslog_level",
                                 "The syslog log level",
                                 opt.syslog_sink_log_level,
                                 &opt.syslog_sink_log_level);
#endif

        op.parse(argc, argv);

        if (help->count() >= 1)
        {
            std::cout << op;
            return 0;
        }

        if (std::filesystem::exists(opt.socket_file))
        {
            std::filesystem::remove(opt.socket_file);
        }

        Server server{opt};
        while (g_signal == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
    }
    catch (const popl::invalid_option& e)
    {
        if (help->count() >= 1)
        {
            std::cout << op;
            return 0;
        }
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
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
