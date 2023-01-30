import socket
import logging


class CustomHandler(logging.Handler):

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
        try:
            self.sock.connect(socket_file)
        except BaseException:
            pass

    def xmlesc(self, data):
        return data.translate(self.table)

    def emit(self, record):
        msg = ("<c>{channel_name}</c><p>{process_id}</p>"
              "<u>{time_us_since_epoch}</u><f>{filename}</f><i>{line_no}</i>"
              "<n>{function_name}</n><m>{log_message}</m>").format(
                channel_name=self.channel_name,
                process_id=record.process,
                time_us_since_epoch=round(record.created * 1e6),
                filename=self.xmlesc(record.filename),
                line_no=record.lineno,
                function_name=self.xmlesc(record.funcName),
                log_message=self.xmlesc(record.msg))
        try:
            self.sock.send(msg.encode())
        except BaseException:
            pass


logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

handler = CustomHandler("/tmp/server.sock", "myorg")
handler.setLevel(logging.DEBUG)

logger.addHandler(handler)

while True:
    logger.debug(input(">>>"))
