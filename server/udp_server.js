const http = require('http');
const fs = require('fs');
const path = require('path');
const express = require('express');
const cors = require('cors');
const { spawn } = require("child_process");
var socketio = require('socket.io').Server;
var io;
var dgram = require('dgram');

exports.listen = function(server) {
    io = new socketio({
        cors: {
            origin: true,
            credentials: true
        }
    });
    io.listen(server);

    io.sockets.on('connection', function(socket) {
        console.log("connected");
        handleCommand(socket);
    });
};

function handleCommand(socket) {

    // Pased string of comamnd to relay
    socket.on('udpCommand', function(data) {
        var errorTimer = setTimeout(function() {
            socket.emit("timeoutError", "setErrorOn");
        }, 1000)
        console.log('udpCommand command: ' + data);

        // Info for connecting to the local process via UDP
        var PORT = 12345;
        var HOST = '127.0.0.1';
        var buffer = new Buffer(data);

        var client = dgram.createSocket('udp4');
        client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
            if (err)
                throw err;
            console.log('UDP message sent to ' + HOST +':'+ PORT);
        });

        client.on('listening', function () {
            var address = client.address();
            console.log('UDP Client: listening on ' + address.address + ":" + address.port);
        });
        // Handle an incoming message over the UDP from the local application.
        client.on('message', function (message, remote) {
            console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);

            var reply = message.toString('utf8').replace(/\0/g, "");
            socket.emit('commandReply', reply);
            clearTimeout(errorTimer);
            client.close();

        });
        client.on("UDP Client: close", function() {
            console.log("closed");
        });
        client.on("UDP Client: error", function(err) {
            console.log("error: ",err);
        })
        // client.on("screenshot", function(){
        //     const SCREENSHOT_DIR = path.join(require('os').homedir(), `webcam-capture`)
        //     fs.readFile(SCREENSHOT_DIR, (err) =>{
        //         if(err){
        //             spawn("mkdir", [SCREENSHOT_DIR]);
        //         }
        //     });
        //     const PICTURE_DIR = `${SCREENSHOT_DIR}/SCREENSHOT_${(new Date().toJSON())}.jpg`;
        //     const camera = spawn("fswebcam", [
        //         "-r 640x480"," --jpeg 85","-D 0",
        //         `${PICTURE_DIR}`
        //         ]
        //     );
        //     camera.on("close",()=>{
        //         let image = new Image();
        //         image.src = PICTURE_DIR;
        //     })
        // })
    });
};