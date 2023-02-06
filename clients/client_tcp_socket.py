import json
import logging
import socket


class ScriptorHandler(logging.Handler):

    def __init__(self, host, port, channel_name, level=logging.NOTSET):
        super().__init__(level)
        self._host = host
        self._port = port
        self._connect()
        self._channel_name = channel_name

    def _connect(self):
        try:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._sock.connect((self._host, self._port))
        except BaseException:
            pass

    def emit(self, record):
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
            self._connect()


logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

handler = ScriptorHandler("127.0.0.1", 12345, "analysis")
handler.setLevel(logging.DEBUG)

logger.addHandler(handler)

while True:
    logger.debug(input(">>> "))
