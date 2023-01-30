# scriptor

[![Actions](https://github.com/bloomen/scriptor/actions/workflows/scriptor-tests.yml/badge.svg?branch=main)](https://github.com/bloomen/scriptor/actions/workflows/scriptor-tests.yml?query=branch%3Amain)

A high-performance logger for Linux using unix file sockets.

## Build

```
sudo apt install libsystemd-dev  # Or similar for your distro
python3 bootstrap.py  # Uses conan to install boost and spdlog to ~/.conan
mkdir build && cd build
cmake ..
make -j
```

## Usage

```
$ ./scriptor --help
scriptor - A high-performance logger for Linux using unix file sockets:
  --help                                Display this help message
  --socket_file arg                     The unix socket filename (required)
  --identity arg (=scriptor)            The identify name
  --filelog_filename arg                The filelog's filename
  --filelog_max_file_size arg (=10485760)
                                        The filelog's max file size
  --filelog_max_files arg (=3)          The filelog's max files
  --filelog_level arg (=0)              The filelog's level
  --systemd_logging                     Enables logging to systemd
  --systemd_level arg (=0)              The systemd log level
  --syslog_logging                      Enables logging to syslog
  --syslog_level arg (=0)               The syslog level
```
For instance:
```
$ ./scriptor --socket_file /tmp/scriptor.sock --identity myorg --filelog_filename /tmp/filelog.txt --systemd_logging --systemd_level 2
```
