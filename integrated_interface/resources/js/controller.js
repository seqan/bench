const path = require('path')
const events = require('events')

var eventEmitter = new events.EventEmitter()

var _SIGNAL = {
    "RUN":false,
    "CANCEL": false,
    "NORM": true,
    "DONE": false
};

var _EVENTS = {
    "INIT": 'INIT',
    "FAILED": 'FAILED'
};
