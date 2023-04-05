var PORT_NUMBER = 8088;

var http = require('http');
var fs   = require('fs');
var path = require('path');
var mime = require('mime');

var server = http.createServer();
server.listen("8080",()=>{
    console.log(`Listening to`,server.address());
});

const udp = require("./udp_server");
udp.listen(server);