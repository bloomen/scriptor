// scriptor - A high-performance logger for Linux using unix file sockets
// Repo: https://github.com/bloomen/scriptor
// Author: Christian Blume
// License: MIT http://www.opensource.org/licenses/mit-license.php

#include <atomic>
#include <csignal>
#include <filesystem>
#include <iostream>

#include <boost/program_options.hpp>

#include "server.h"

using namespace scriptor;
namespace po = boost::program_options;

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
        Options opt;

        po::options_description desc{"scriptor - A high-performance logger for "
                                     "Linux using unix file sockets"};
        desc.add_options()("help", "Display this help message")(
            "socket_file",
            po::value<std::string>(&opt.socket_file)->required(),
            "The unix socket filename (required)")(
            "identity",
            po::value<std::string>(&opt.identity)->default_value(opt.identity),
            "The identity name")

            // file logging
            ("filelog_filename",
             po::value<std::string>(&opt.file_sink_filename),
             "The filelog's filename")(
                "filelog_max_file_size",
                po::value<std::size_t>(&opt.file_sink_max_file_size)
                    ->default_value(opt.file_sink_max_file_size),
                "The filelog's max file size")(
                "filelog_max_files",
                po::value<std::size_t>(&opt.file_sink_max_files)
                    ->default_value(opt.file_sink_max_files),
                "The filelog's max files")(
                "filelog_level",
                po::value<int>(&opt.file_sink_log_level)
                    ->default_value(opt.file_sink_log_level),
                "The filelog's level")

            // systemd logging
            ("systemd_logging",
             po::bool_switch(&opt.systemd_sink_use)
                 ->default_value(opt.systemd_sink_use),
             "Enables logging to systemd")(
                "systemd_level",
                po::value<int>(&opt.systemd_sink_log_level)
                    ->default_value(opt.systemd_sink_log_level),
                "The systemd log level")

            // syslog logging
            ("syslog_logging",
             po::bool_switch(&opt.syslog_sink_use)
                 ->default_value(opt.syslog_sink_use),
             "Enables logging to syslog")(
                "syslog_level",
                po::value<int>(&opt.syslog_sink_log_level)
                    ->default_value(opt.syslog_sink_log_level),
                "The syslog level");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        po::notify(vm);

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
