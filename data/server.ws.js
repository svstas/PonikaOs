#!/bin/env node
console.log("Start here");
var { exec } = require('child_process');
function reset() {

    exec('/usr/bin/gpio write 8 1',(err, stdout, stderr) => {});
    setTimeout(function() {exec('/usr/bin/gpio write 8 0',(err, stdout, stderr) => {});},300);

}
reset();
/* reboot embedded system 
exec('/usr/bin/gpio write 8 1',(err, stdout, stderr) => {});

setTimeout(function() {
exec('/usr/bin/gpio write 8 0',(err, stdout, stderr) => {});
},200);
*/

const util = require('util')
var EventSource = require('eventsource');
var WebSocketClient = require('websocket').client;
//var WebSocketClient = require('ws').client;
//var client;
                                                         
started = false;

const { v4: uuidv4 } = require('uuid');

function display(state) {

exec('/usr/bin/gpio write 3 '+parseInt(state),(err, stdout, stderr) => {});

}

const WebSocket = require('ws');
var wss;
// = new WebSocket.Server({ port: 7071 });
const clients = new Map();

function sendcl(msg) {
console.log(">>>>>>>> "+msg);
	[...clients.keys()].forEach((client) => {
        client.send(msg);});

}

function websock(xstarted) {

var client = new WebSocketClient();
client.on('connectFailed', function(error) {console.log('Connect Error: ' + error.toString());});

    client.on('connect', function(xc) {
    console.log('Pastiralla WorkShop connected');
    xc.on('error', function(error) {console.log("Connection Error: " + error.toString());});
    xc.on('close', function() {console.log('echo-protocol Connection Closed'); setTimeout(websock.bind(null, 1), 2500);
});

    xc.on('message', function(message) {
        if (message.type === 'utf8') {
//            console.log("Received: '" + message.utf8Data + "'");
	    sendcl(message.utf8Data);
        }
//	console.log("Rec: "+message);
//	console.log(util.inspect(message, {showHidden: false, depth: null, colors: true}))
    });

if (chkwrite) clearInterval(chkwrite);
var xcommand = "";

var chkwrite = setInterval(() => {if (xcommand) xc.sendUTF(xcommand); xcommand="";}, 50);
//    xc.sendUTF("ls");

var es = new EventSource('http://192.168.1.28/events')
console.log('add events');
es.addEventListener('m', function (e) {
//  console.log(">>"+e.data);
  sendcl(e.data);
  if (e.data.substring(0,7)=='display') {
    dEn = parseInt(e.data.substring(8));
    display(dEn);
}
})

if (!started) {
console.log("START!!!");
started = true;
display(1);

wss = new WebSocket.Server({ port: 7071 });

wss.on('connection', (ws) => {
    const id = uuidv4();
    const color = Math.floor(Math.random() * 360);
    const metadata = { id, color };
    display(1);
    clients.set(ws, metadata);
    xc.sendUTF("y1:10|");
    setTimeout(function() {xc.sendUTF("xmass 500");},500);
    ws.on('message', (messageAsString) => {

//      const message = JSON.parse(messageAsString);
      const message = messageAsString;
      const metadata = clients.get(ws);
	if (!message.length) return;
      message.sender = metadata.id;
      message.color = metadata.color;
//      client.sendUTF(message);

	xcommand = message;
      console.log("cmd>"+message+"|");
//  const outbound = JSON.stringify(message);
//  const outbound = ">>>"+message;
});
   ws.on("close", () => {
      clients.delete(ws);
    });
console.log("wss started");
});


}
//if (wss) wss.close();
//console.log('uh ty');



if (!xstarted) {
xstarted = 1;
xc.sendUTF("q185|");
}

/*
if (!xstarted) {
const readline = require('readline');
const rl = readline.createInterface({
  input: process.stdin, 
  output: process.stdout
});

const start = async () =>{
    started = true;
    for await (const line of rl) {
	xc.sendUTF(line);

    }
    }
 start();
}
*/






});



client.connect('ws://192.168.1.28/ws', ['arduino']);

};

//websock(false);






var http = require('http').createServer(handler);
var fs = require('fs');

//var io = require('socket.io')(http);



//http.createServer(function (req, res) {
//  res.writeHead(200, {'Content-Type': 'text/html'});
//  res.end(index);
//}).listen(8080);


function debug(a) {
console.log("dbg> "+a);
}


function handler (req, res) { //create server
var url = "";
if (req.url=='/') url="index.html"; url = req.url+url;
if (req.url=="/") {debug("Got client!");    reset();}


if (req.url.substring(0,6)=="/reset") {

//    exec('/usr/bin/gpio write 3 1',(err, stdout, stderr) => {});
 
    websock(false); 
    console.log("Display ON");
}


//console.log(url);

  fs.readFile(__dirname + '/public'+url, function(err, data) { //read file index.html in public folder
    if (err) {
      res.writeHead(404, {'Content-Type': 'text/html'}); //display 404 on error
      return res.end("404 Not Found");
    }

        ext = url.substring(url.lastIndexOf('.')+1);
        var contt = "text/html";
        switch(ext) {
                        case "js" : contt = "text/javascript";break;
                        case "css" : contt = "text/css";break;
        }
    res.writeHead(200, {'Content-Type': contt}); //write HTML
    res.write(data); //write data from index.html
    return res.end();

  });
}

http.listen(8080);

//    if (url=="/index.html") 
/*
function myfunc() {
exec('/usr/bin/gpio write 3 0',(err, stdout, stderr) => {});
}

setTimeout(myfunc,10000);
*/