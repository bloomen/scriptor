import socket
import logging


class ScriptorHandler(logging.Handler):

    def __init__(self, socket_file, channel_name, level=logging.NOTSET):
        super().__init__(level)
        self.table = str.maketrans({
            "<": "&lt;",
            ">": "&gt;",
            "&": "&amp;",
            "'": "&apos;",
            '"': "&quot;",
        })
        self.channel_name = self.xmlesc(channel_name)
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.connect(socket_file)

    def xmlesc(self, data):
        return data.translate(self.table)

    def emit(self, record):
        msg = ("<c>{channel_name}</c><s>{seconds_since_epoch}</s>"
               "<l>{log_level}</l><p>{process}</p><f>{filename}</f>"
               "<i>{line_no}</i><n>{function_name}</n><m>{log_message}</m>"
               ).format(channel_name=self.channel_name,
                       seconds_since_epoch=record.created,
                       log_level=int(record.levelno / 10),
                       process=record.process,
                       filename=self.xmlesc(record.filename),
                       line_no=record.lineno,
                       function_name=self.xmlesc(record.funcName),
                       log_message=self.xmlesc(record.msg))
        self.sock.send(msg.encode())


logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

handler = ScriptorHandler("/tmp/server.sock", "myorg")
handler.setLevel(logging.DEBUG)

logger.addHandler(handler)

while True:
    logger.debug(input(">>> "))
