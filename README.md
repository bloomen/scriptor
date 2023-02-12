# scriptor

[![Actions](https://github.com/bloomen/scriptor/actions/workflows/scriptor-tests.yml/badge.svg?branch=main)](https://github.com/bloomen/scriptor/actions/workflows/scriptor-tests.yml?query=branch%3Amain)

A high-performance logger using unix/tcp sockets.
Requires a C++17 compliant compiler.
Tested with Clang, GCC, and Visual Studio.

## Architecture

scriptor is a server process that accepts unix/tcp socket connections. Once connected, clients
can send json messages in the format defined below. scriptor then parses the json and
forwards it to the registered loggers.

![architecture](https://raw.githubusercontent.com/bloomen/scriptor/main/architecture.png)

scriptor currently supports the following loggers:

* Rotating file
* Systemd (Linux only)
* Syslog (Linux only)
* Windows EventLog (Windows only)

## Build & Install

```
(sudo apt install libsystemd-dev)  # If built for Linux. Adjust for your distro
python3 bootstrap.py  # Uses conan to install asio, nlohmann_json, spdlog to ~/.conan
cmake -DCMAKE_BUILD_TYPE=Release .
cmake --build . -j --config Release
cmake --install . --config Release
```

Optionally configure the systemd service for Linux:
```
sudo cp systemd/scriptor.service /etc/systemd/system/
sudo chmod 664 /etc/systemd/system/scriptor.service
sudo systemctl daemon-reload
sudo systemctl start scriptor.service
```
You'd want to edit the `scriptor.service` file to fit your needs.

## Usage

```
$ ./scriptor --help
scriptor - A high-performance logger using unix/tcp sockets:
  -h, --help                          Display this help message
  --socket_file arg                   The unix socket filename
  --socket_address arg                The tcp socket address
  --socket_port arg (=12345)          The tcp socket port
  --identity arg (=scriptor)          The identity name
  --threads arg (=4)                  The number of producer threads
  --filelog_filename arg              The filelog filename
  --filelog_max_file_size arg (=10485760)
                                      The filelog max file size
  --file_sink_max_files arg (=3)      The filelog max files
  --filelog_level arg (=0)            The filelog log level (0>=level<=5)
  --systemd_logging                   Enables logging to systemd (Linux only)
  --systemd_level arg (=0)            The systemd log level (0>=level<=5)
  --syslog_logging                    Enables logging to syslog (Linux only)
  --syslog_level arg (=0)             The syslog log level (0>=level<=5)
```
Logging to systemd and syslog is only available on Linux.

For instance:
```
$ ./scriptor --socket_file /tmp/scriptor.sock --identity myorg\
  --filelog_filename /var/log/scriptor.log --systemd_logging --systemd_level 2
```
This starts a server listening for connections on the given file socket.
Any number of clients can then connect and send logs to scriptor. Each log
is forwarded to both file and systemd. A log must follow this JSON format:
```
{"c":"channel_name","s":seconds_since_epoch,"l":log_level,"p":process_id,\
    "t":thread_id,"f":"filename","i":line_no,"n":"function","m":"message"}
```
None of the fields are mandatory but a log must start with a `{` and
end with a `}`. For a concrete example:
```
{"c":"analysis","s":1675153178.487972,"l":1,"p":5887,\
    "f":"client.py","i":48,"n":"compute()","m":"hello there"}
```
The log level is a number between 0 (trace) and 5 (critical).
To see how to talk to scriptor from python check the examples under the `clients` directory.
