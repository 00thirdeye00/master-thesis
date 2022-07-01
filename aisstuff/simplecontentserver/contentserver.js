#!/usr/bin/env node

//debug logging
console.debug = () => {};

// Setup Express as http server
const options = {
  keepAlive: false,
  noDelay: true
}
const http_port=8081;

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

const stime = 10*1000; // 30 seconds
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

// Start the http server
server.listen(http_port, () => {
  console.log('listening on *: ' + http_port);
});


// Setup coap server

const coap_port=15683
const coap = require('coap');
const coapserver4 = coap.createServer();
const coapserver6 = coap.createServer({ type: 'udp6' });

// coap server handle incoming request
const handleReq = (req, res) => {
  res.end(chunk512b);
};

coapserver4.on('request', handleReq);
coapserver6.on('request', handleReq);


// Start listener and send client request to test that server is running

const handleRes = (res) => {
  res.pipe(process.stdout);
  res.on('end', () => {});
};

coapserver4.listen(coap_port, () => {
  console.log('listening on coap port: ' + coap_port);
  const req = coap.request('coap://localhost:'+coap_port+'/anders4')
  req.on('response', handleRes);
  req.end()
})

coapserver6.listen(coap_port, () => {
  console.log('listening on coap port: ' + coap_port);
  const req = coap.request('coap://[::1]:'+coap_port+'/anders6')
  req.on('response', handleRes);
  req.end()
})
