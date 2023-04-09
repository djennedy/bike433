var PORT_NUMBER = 8088;

var http = require('http');
var fs   = require('fs');
var path = require('path');
var mime = require('mime');
const express = require("express");
const app = express();


/*
 * Create the static web server
 */
var server = http.createServer(function(request, response) {
    var filePath = false;

    if (request.url === '/') {
        filePath = 'dist/index.html';
    } else {
        filePath = 'dist' + request.url;
    }

    var absPath = './' + filePath;
    serveStatic(response, absPath);
});

server.listen(PORT_NUMBER, function() {
    console.log("Server listeneing on port " + PORT_NUMBER);
});

function serveStatic(response, absPath) {
    fs.exists(absPath, function(exists) {
        if (exists) {
            fs.readFile(absPath, function(err, data) {
                if (err) {
                    send404(response);
                } else {
                    sendFile(response, absPath, data);
                }
            });
        } else {
            send404(response);
        }
    });
}

function send404(response) {
    response.writeHead(404, {'Content-Type': 'text/plain'});
    response.write('Error 404: resource not found.');
    response.end();
}

function sendFile(response, filePath, fileContents) {
    response.writeHead(
        200,
        {"content-type": mime.lookup(path.basename(filePath))}
    );
    response.end(fileContents);
}

app.use(express.static(path.join(__dirname)));

app.get('/images/:file',function(req,res){
    const file = req.params.file;
    console.log("XYZ",path.join(__dirname, 'public',file));
    fs.readFile(path.join(__dirname, 'public',file), (err, data) => {
        if (err) {
            res.writeHead(500);
            res.end(`Error loading image: ${err}`);
            return;
        }

        res.writeHead(200, {
            'Content-Type': 'image/jpeg',
            'Access-Control-Allow-Origin': '*',
        });
        res.end(data);
    });
})

app.listen(3000, () => {
    console.log('Image server started on port 3000');
});

const udp = require("./udp_server");
udp.listen(server);