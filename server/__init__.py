from flask import Flask
from config import Config
from flask_socketio import SocketIO


class UserError(ValueError):
    pass


class ServerError(ValueError):
    CODE_EXTERNAL_PROGRAM = 10
    CODE_RESULT_PARSING = 11

    def __init__(self, code, message):
        super(ValueError, self).__init__(message)
        self.code = code


app = Flask(__name__)
app.config.from_object(Config)
socketio = SocketIO(app)

from server import routes
