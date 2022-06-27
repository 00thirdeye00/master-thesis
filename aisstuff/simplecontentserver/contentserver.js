#!/usr/bin/env node

//debug logging
console.debug = () => {};

// Setup Express as web server
const options = {
  keepAlive: false,
  noDelay: true
}
const portNo=8081

const express = require('express');
const app = express();
const server = require('http').createServer(options, app);

// const bodyParser = require('body-parser');

/* 128 bytes */
const chunk128b = Buffer.from("01234567891123456789212345678931234567894123456789512345678961234567897123456789812345678991234567890123456789112345678921234567");

/* 512 bytes */
const chunk512b = Buffer.concat([chunk128b, chunk128b, chunk128b, chunk128b]);


/* 1024 bytes */
const chunk1kb = Buffer.concat([chunk128b, chunk128b, chunk128b, chunk128b, chunk128b, chunk128b, chunk128b, chunk128b]);


// Setup body parser and debug logging
// app.use(bodyParser.text({type: 'text/plain'}));
// app.use(bodyParser.json()); // support json encoded bodies
// app.use(bodyParser.urlencoded({ extended: true })); // support encoded bodies

const stime = 30*1000; // 30 seconds
const sleep = ms => new Promise(res => setTimeout(res, ms));


// Static Content
app.get('/:nchunks', async (req, res) => {
  const {nchunks = 1} = req.params;
  const chunk = chunk512b;
  const len=nchunks*chunk.length;
  res.setHeader("Content-Type", 'text/plain');
  res.setHeader("Content-Length", len.toString());
  res.setHeader("Connection", "close");
  res.setHeader("Transfer-Encoding", 'identity');


  await res.write(chunk);
  for (let i = 1; i < nchunks; i++) {
    await sleep(stime);
    await res.write(chunk);
  }

  res.end();
});

// Start the Server
server.listen(portNo, () => {
  console.log('listening on *: ' + portNo);
});
