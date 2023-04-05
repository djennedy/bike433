var PORT_NUMBER = 8088;

var http = require('http');
var fs   = require('fs');
var path = require('path');
var mime = require('mime');
const express = require("express");


var finalhandler = require('finalhandler');
var serveStatic = require('serve-static');

var serve = serveStatic("./dist");

var server = http.createServer(function(req, res) {
    // Set CORS headers
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Request-Method', '*');
    res.setHeader('Access-Control-Allow-Methods', 'OPTIONS, GET');
    res.setHeader('Access-Control-Allow-Headers', '*');

    var done = finalhandler(req, res);
    serve(req, res, done);
});
server.listen("8080",()=>{console.log("Listening at port", 8080)});



const udp = require("./udp_server");
udp.listen(server);