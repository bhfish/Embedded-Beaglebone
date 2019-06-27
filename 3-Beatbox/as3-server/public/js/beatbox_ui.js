"use strict";
// Client-side interactions with the browser.

// Websocket connection to server
var socket = io.connect();

// Make connection to server when web page is fully loaded.
$(document).ready(function() {
	console.log("Document loaded");
	
	// Send message to request some (one-shot) updates:
	//sendUdpCommand('update');
	window.setInterval(function() {
		sendUdpCommand('update');
	}, 1000);

	

	// Setup a repeating function (every 1s)
	window.setInterval(function() {
		sendRequest('uptime');
		window.uptimeTimer = setTimeout(function(){
	  	 showError('NodeJS server is not working!');
		}, 1000);
	}, 1000);

 	//setup control button function
	$('#volumeUp').click(function(){
		sendUdpCommand("volumeUp");
	});
	$('#volumeDown').click(function(){
		sendUdpCommand("volumeDown");
	});
	$('#tempoUp').click(function(){
		sendUdpCommand("tempoUp");
	});
	$('#tempoDown').click(function(){
		sendUdpCommand("tempoDown");
	});
	$('#mode_none').click(function(){
		sendUdpCommand("none");
	});
	$('#mode_rock').click(function(){
		sendUdpCommand("rock");
	});
	$('#mode_custom').click(function(){
		sendUdpCommand("custom");
	});
	$('#hihat').click(function(){
		sendUdpCommand("hihat");
	});
	$('#snare').click(function(){
		sendUdpCommand("snare");
	});
	$('#base').click(function(){
		sendUdpCommand("base");
	});

	// Handle data coming back from the server
	socket.on('fileContents', function(result) {
		var fileName = result.fileName;
		var contents = result.contents;
		//console.log("fileContenst callback: fileName " + fileName + ", contents: " + contents);		
		if(fileName=='uptime'){
			clearTimeout(uptimeTimer);
			contents = parseUptime(contents);
			$('#uptime').html(contents);
			
		}
		else{
			console.log("Unknown DOM object: " + fileName);
			return;
		}		
	});	

	socket.on('beatboxStatus', function(result) {
		var status = result.split(",");
    		var mode = parseInt(status[0]);
    		var volume = parseInt(status[1]);
    		var tempo = parseInt(status[2]);
		console.log("beatbox status: mode " + mode + ", volume: " + volume + ", tempo: " + tempo);
		switch(mode){
			case 0:
				$('#currentMode').html("<b>Rock</b>");
				break;
			case 1:
				$('#currentMode').html("<b>Custom</b>");
				break;
			case 2:
				$('#currentMode').html("<b>None</b>");
				break;
			default:
				break;
		}
		$('#volumeValue').val(volume);
		$('#tempoValue').val(tempo+" BPM");
		//clearTimeout(updatetimer);
	});	

	socket.on('beatboxVolume', function(result) {
    		var volume = parseInt(result);
		console.log("beatbox volume: " + volume);
		$('#volumeValue').val(volume);
	});

	socket.on('beatboxTempo', function(result) {
    		var tempo = parseInt(result);
		console.log("beatbox tempo: " + tempo);
		$('#tempoValue').val(tempo+" BPM");
	});
	
	socket.on('beatboxMode', function(result) {
    		var mode = parseInt(result);
		console.log("beatbox mode: " + mode);
		switch(mode){
			case 0:
				$('#currentMode').html("<b>Rock</b>");
				break;
			case 1:
				$('#currentMode').html("<b>Custom</b>");
				break;
			case 2:
				$('#currentMode').html("<b>None</b>");
				break;
			default:
				break;
		}
	});

	socket.on('udpError', function(err) {
		showError(err);
	});
});

function sendRequest(file) {
	console.log("Requesting '" + file + "'");
	socket.emit('proc', file);
}

function parseUptime(str) {
	var totalSeconds = parseFloat(str);
	var hours = Math.floor(totalSeconds / 3600);
	var mins = Math.floor((totalSeconds - 3600*hours)/60);
	var seconds = Math.floor(totalSeconds-hours*3600-mins*60);
	return hours + " hours "+ mins +" minutes "+seconds+" seconds";
}

function sendUdpCommand(message) {
	socket.emit('udp', message);
};

function showError(err){
	console.log(err);
	$('#error').html("<b>Error: </b>"+err);
	if(!$('#error').is(":visible")){
		$('#error').show();
		setTimeout(function(){
	  	    $('#error').hide();
		}, 10000);
	}
}
