#include <atomic>
#include <filesystem>
#include <iostream>

#include "popl.hpp"

#include "scriptor.h"
#include "server.h"

namespace scriptor
{

namespace
{

std::atomic<int> g_signal{0};

}

void
stop(const int signal)
{
    g_signal = signal;
}

int
run(int argc, char** argv)
{
    g_signal = 0;
    popl::OptionParser op{
        "scriptor - A high-performance logger using unix/tcp sockets"};
    auto help = op.add<popl::Switch>("h", "help", "Display this help message");
    try
    {
        Options opt;

        op.add<popl::Value<std::string>>("",
                                         "socket_file",
                                         "The unix socket filename",
                                         opt.socket_file,
                                         &opt.socket_file);
        op.add<popl::Value<std::string>>("",
                                         "socket_address",
                                         "The tcp socket address",
                                         opt.socket_address,
                                         &opt.socket_address);
        op.add<popl::Value<asio::ip::port_type>>("",
                                                 "socket_port",
                                                 "The tcp socket port",
                                                 opt.socket_port,
                                                 &opt.socket_port);
        op.add<popl::Value<std::string>>(
            "", "identity", "The identity name", opt.identity, &opt.identity);
        op.add<popl::Value<std::size_t>>("",
                                         "threads",
                                         "The number of producer threads",
                                         opt.n_threads,
                                         &opt.n_threads);

        // file logging
        op.add<popl::Value<std::string>>("",
                                         "filelog_filename",
                                         "The filelog filename",
                                         opt.file_sink_filename,
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
                                 "The filelog log level (0>=level<=5)",
                                 opt.file_sink_log_level,
                                 &opt.file_sink_log_level);

        // systemd logging
        op.add<popl::Switch>("",
                             "systemd_logging",
                             "Enables logging to systemd (Linux only)",
                             &opt.systemd_sink_use);
        op.add<popl::Value<int>>("",
                                 "systemd_level",
                                 "The systemd log level (0>=level<=5)",
                                 opt.systemd_sink_log_level,
                                 &opt.systemd_sink_log_level);

        // syslog logging
        op.add<popl::Switch>("",
                             "syslog_logging",
                             "Enables logging to syslog (Linux only)",
                             &opt.syslog_sink_use);
        op.add<popl::Value<int>>("",
                                 "syslog_level",
                                 "The syslog log level (0>=level<=5)",
                                 opt.syslog_sink_log_level,
                                 &opt.syslog_sink_log_level);

        // eventlog logging
        op.add<popl::Switch>("",
                             "eventlog_logging",
                             "Enables logging to eventlog (Windows only)",
                             &opt.eventlog_sink_use);
        op.add<popl::Value<int>>("",
                                 "eventlog_level",
                                 "The eventlog log level (0>=level<=5)",
                                 opt.eventlog_sink_log_level,
                                 &opt.eventlog_sink_log_level);

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

} // namespace scriptor
