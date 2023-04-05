"use strict";
/*
 * Respond to commands over a websocket to relay UDP commands to a local program
 * Adapted from example code in class
 */

var socketio = require('socket.io');
var io;

var fs = require('fs');

var dgram = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');

	io.sockets.on('connection', function(socket) {
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
	});

    socket.on('getUptime', function() {
        var uptime = fs.readFileSync('/proc/uptime', 'utf-8');
        var uptimeSplit = uptime.split(" ");
        socket.emit('uptimeReply', uptimeSplit[0]);
    })
};