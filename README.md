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
  --identity arg (=scriptor)            The identity name
  --filelog_filename arg                The filelog filename
  --filelog_max_file_size arg (=10485760)
                                        The filelog max file size
  --filelog_max_files arg (=3)          The filelog max files
  --filelog_level arg (=0)              The filelog level
  --systemd_logging                     Enables logging to systemd
  --systemd_level arg (=0)              The systemd log level
  --syslog_logging                      Enables logging to syslog
  --syslog_level arg (=0)               The syslog level
```
For instance:
```
$ ./scriptor --socket_file /tmp/scriptor.sock --identity myorg\
  --filelog_filename /tmp/filelog.txt --systemd_logging --systemd_level 2
```
This starts a server listening for connections on the given file socket.
Any number of clients can then connect and send logs to scriptor. Each log
is forwarded to both file and systemd. A log must follow this XML format:
```
<c>channel_name</c><s>seconds_since_epoch</s><l>log_level</l><p>process</p>\
  <t>thread</t><f>filename</f><i>line_no</i><n>function_name</n><m>log_message</m>
```
Note that only channel_name (`<c>`) and log_message (`<m>`) are mandatory and **must**
be supplied at the start and end of the log, respectively. For a concrete example:
```
<c>myorg</c><s>1675153178.487972</s><l>1</l><p>5887</p><f>client.py</f>\
  <i>48</i><n>compute()</n><m>hello there</m>
```
To see how to talk to scriptor from python check the `client.py` example code.
