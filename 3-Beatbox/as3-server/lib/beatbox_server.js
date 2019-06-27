"use strict";
/*
 * Respond to commands over a websocket to control beatbox and access /proc
 */

var fs   = require('fs');
var socketio = require('socket.io');
var io;
var dgram = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');
	
	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {
	
	socket.on('proc', function(fileName) {
		var absPath = "/proc/" + fileName;
		console.log('accessing ' + absPath);
		
		fs.exists(absPath, function(exists) {
			if (exists) {
				fs.readFile(absPath, function(err, fileData) {
					if (err) {
						emitSocketData(socket, fileName, 
								"ERROR: Unable to read file " + absPath);
					} else {
						emitSocketData(socket, fileName, 
								fileData.toString('utf8'));
					}
				});
			} else {
				emitSocketData(socket, fileName, 
						"ERROR: File " + absPath + " not found.");
			}
		});
		//clearTimeout(errorTimer);
	});

	socket.on('udp', function(command) {
		var errorTimer = setTimeout(function() {
			socket.emit("udpError", "BeatBox application did not respond!");
		}, 1000);
		console.log('udp command: ' + command);
		var PORT = 12345;
		var HOST = '127.0.0.1';
		var buffer = new Buffer(command);
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
		    clearTimeout(errorTimer);
		    console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
		    
		    var reply = message.toString('utf8');
		    var replies = reply.split(":");
		    var replyType = replies[0];
		    switch(replyType){
			case "status":
				socket.emit('beatboxStatus', replies[1]);
				break;
			case "volume":
				socket.emit('beatboxVolume', replies[1]);
				break;
			case "tempo":
				socket.emit('beatboxTempo', replies[1]);
				break;
			case "mode":
				socket.emit('beatboxMode', replies[1]);
				break;
			default:
				socket.emit('beatboxPlay', reply);
				break;
			}
		    
		    
		    client.close();
		    
		});

		client.on("UDP Client: close", function() {
		    console.log("closed");
		});
		client.on("UDP Client: error", function(err) {
		    console.log("error: ",err);
		});
		
	});

};

function emitSocketData(socket, fileName, contents) {
	var result = {
			fileName: fileName,
			contents: contents
	}
	socket.emit('fileContents', result);	
}
