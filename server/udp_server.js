const {Server} = require('socket.io');
var io;
const { spawn } = require('child_process');
var fs = require('fs');
var path = require("path");
var dgram = require('dgram');

exports.listen = function(server) {
    io = new Server(server);

    io.sockets.on('connection', function(socket) {
        handleCommand(socket);
    });
};

function handleCommand(socket) {
    // Pased string of comamnd to relay
    socket.on('udpCommand', function(data,callback) {
        var errorTimer = setTimeout(function() {
            socket.emit("timeoutError", "setErrorOn");
        }, 1000)
        console.log('udpCommand command: ' + data);

        // Info for connecting to the local process via UDP
        var PORT = 12345;
        var HOST = '127.0.0.1';
        var buffer = new Buffer.from(data);


        var client = dgram.createSocket('udp4');
        client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
            if (err)
                throw err;
            console.log('UDP message sent to ' + HOST +':'+ PORT);
        });

        client.on('listening', function () {
            var address = client.address();
            // console.log('UDP Client: listening on ' + address.address + ":" + address.port);
        });
        // Handle an incoming message over the UDP from the local application.
        client.on('message', function (message, remote) {
            console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
            console.log("Message before filtering:", message.toString());
            var reply = message.toString('utf8').replace(/\0/g, "");
            console.log(`Message received: ${reply}`);
            callback(reply);
            clearTimeout(errorTimer);
            client.close();

        });
        client.on("UDP Client: close", function() {
            console.log("closed");
        });
        client.on("UDP Client: error", function(err) {
            console.log("error: ",err);
        })
    });

    socket.on("webcam-capture", function(callback){
        const SCREENSHOT_DIR = path.join(__dirname, `images`);
        fs.readFile(SCREENSHOT_DIR, (err) =>{
            if(err){
                spawn("mkdir", [SCREENSHOT_DIR]);
            }
        });
        const PICTURE_NAME = `SCREENSHOT_${(new Date().toJSON())}.jpg`;
        const camera = spawn("fswebcam", [
                "-r 640x480"," --jpeg 85","-D 0",
                `${path.join(SCREENSHOT_DIR,PICTURE_NAME)}`
            ]
        );
        camera.on("close",()=>{
            callback(`localhost:3000/images/`+PICTURE_NAME);
        })
    })

};