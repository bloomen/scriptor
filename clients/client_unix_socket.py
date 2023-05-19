import json
import logging
import socket


class ScriptorHandler(logging.Handler):

    def __init__(self, socket_file, channel_name, level=logging.NOTSET):
        super().__init__(level)
        self._socket_file = socket_file
        self._channel_name = channel_name

    def _close(self):
        try:
            self._sock.shutdown(socket.SHUT_WR)
            self._sock.close()
        except BaseException:
            pass

    def _connect(self):
        self._close()
        try:
            self._sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self._sock.connect(self._socket_file)
            return True
        except BaseException:
            return False

    def emit(self, record):
        if not self._connect():
            return
        msg = dict(c=self._channel_name,
                   s=record.created,
                   l=int(record.levelno / 10),
                   p=record.process,
                   f=record.filename,
                   i=record.lineno,
                   n=record.funcName,
                   m=record.msg)
        bytes = json.dumps(msg, separators=(',', ':')).encode()
        try:
            self._sock.send(bytes)
        except BaseException:
            pass
        finally:
            self._close()


logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

handler = ScriptorHandler("/tmp/scriptor.sock", "analysis")
handler.setLevel(logging.DEBUG)

logger.addHandler(handler)

while True:
    logger.debug(input(">>> "))
