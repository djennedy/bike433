const http = require('http');
const fs = require('fs');
const path = require('path');
const express = require('express');
const cors = require('cors');

const mpdPath =("/dev/shm/dash/");

const app = express();

app.options('*', cors());


app.get('/:file',function(req,res){
    console.log(req.url);
    const file = req.params.file;
    fs.readFile(path.join(mpdPath,file), (err, data) => {
      if (err) {
        res.writeHead(500);
        res.end(`Error loading live.mpd: ${err}`);
        return;
      }

      res.writeHead(200, {
        'Content-Type': 'application/dash+xml',
        'Access-Control-Allow-Origin': '*',
      });
      res.end(data);
    });
})


app.listen(8080, () => {
  console.log('Server running at http://localhost:8080/');
});