import socket
import logging


class ScriptorHandler(logging.Handler):

    def __init__(self, socket_file, channel_name, level=logging.NOTSET):
        super().__init__(level)
        self._table = str.maketrans({
            "<": "&lt;",
            ">": "&gt;",
            "&": "&amp;",
            "'": "&apos;",
            '"': "&quot;",
        })
        self._channel_name = self._xmlesc(channel_name)
        self._socket_file = socket_file
        self._connect()

    def _connect(self):
        try:
            self._sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self._sock.connect(self._socket_file)
        except BaseException:
            pass

    def _xmlesc(self, data):
        return data.translate(self._table)

    def emit(self, record):
        msg = ("<c>{channel_name}</c><s>{seconds_since_epoch}</s>"
               "<l>{log_level}</l><p>{process}</p><f>{filename}</f>"
               "<i>{line_no}</i><n>{function_name}</n><m>{log_message}</m>"
               ).format(channel_name=self._channel_name,
                        seconds_since_epoch=record.created,
                        log_level=int(record.levelno / 10),
                        process=record.process,
                        filename=self._xmlesc(record.filename),
                        line_no=record.lineno,
                        function_name=self._xmlesc(record.funcName),
                        log_message=self._xmlesc(record.msg))
        try:
            self._sock.send(msg.encode())
        except BaseException:
            self._connect()


logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

handler = ScriptorHandler("/tmp/scriptor.sock", "myorg")
handler.setLevel(logging.DEBUG)

logger.addHandler(handler)

while True:
    logger.debug(input(">>> "))
