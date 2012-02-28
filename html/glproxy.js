var canvas_resize = function(){
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
}

var draw_canvas = function(nodes){
  ctx = canvas.getContext('2d');
  ctx.clearRect(0,0,canvas.width, canvas.height)
  ctx.fillStyle = "rgb(200,0,0)";
  console.log(nodes);
  for (i in nodes){
    console.log(i);
    var n = nodes[i];
      ctx.fillRect (n.x-10,n.y-10,20,20);
  }
}

function readCommand(buf) {
	var cmd = '';
	for (var i = 0; i < buf.byteLength; i++) {
		if (buf[i] == 0)
			break
		cmd += String.fromCharCode(buf[i])
	}
	cmd = cmd[0].toLowerCase() + cmd.substring(1);
    return cmd;
}

function uint(buf) {
	var n = 0;
	n = buf[0];
	n += buf[1] << 8;
	n += buf[2] << 16;
	n += buf[3] << 24;
	return n
}

function ushort(buf) {
	var n = 0;
	n = buf[0];
	n += buf[1] << 8;
	return n
}

function cstr(buf) {
	var cmd = '';
	for (var i = 0; i < buf.byteLength; i++)
		cmd += String.fromCharCode(buf[i])
    return cmd;
}

var data;
var gl_textures = {}
var gl_texture_idx = 0;
var gl_shaders = {}
var gl_shader_idx = 0;
var gl_programs = {}
var gl_program_idx = 0;
var gl_buffers = {}
var gl_buffer_idx = 0;
var socket_recv = function(msg){
	//console.log('received message');
	//console.log(msg.data);
	var buf = new Uint8Array(msg.data).subarray(20);
	var cmd = readCommand(buf);
	//console.log('received cmd ' + cmd);
	buf = buf.subarray(cmd.length + 1);
	data = buf;
	var iarg = uint(buf);
	//console.log('get ' + iarg);
	buf = buf.subarray(4);

	var args = new Array();
	for ( i = 0; i < iarg; i++ ) {
		var size = uint(buf);
		buf = buf.subarray(4);
		args.push(buf.subarray(0, size));
		buf = buf.subarray(size);
	}

	console.log(cmd);
	console.log(args);

	// fix some calls
	switch (cmd) {
		case 'bindAttribLocation':
			gl.bindAttribLocation(uint(args[0]), uint(args[1]), cstr(args[2]));
			break;
		case 'blendFunc':
			gl.blendFunc(uint(args[0]), uint(args[1]));
			break;
		case 'activeTexture':
			gl.activeTexture(uint(args[0]));
			break;
		case 'genTexture':
			gl_textures[gl_texture_idx] = gl.createTexture();
			gl_texture_idx++;
			break;
		case 'genShader':
			gl_shaders[gl_shader_idx] = gl.createShader();
			gl_shader_idx++;
			break;
		case 'genProgram':
			gl_programs[gl_program_idx] = gl.createProgram();
			gl_program_idx++;
			break;
		case 'genShader':
			gl_buffers[gl_buffer_idx] = gl.createBuffer();
			gl_buffer_idx++;
			break;
		case 'bindBuffer':
			idx = uint(args[0]);
			gl.bindBuffer(gl_buffers[idx]);
			break;
		case 'bindTexture':
			idx = uint(args[1]);
			gl.bindTexture(uint(args[0]), gl_textures[idx]);
			break;
		case 'useProgram':
			idx = uint(args[0]);
			gl.useProgram(gl_programs[idx]);
			break;
		case 'compileShader':
			idx = uint(args[0]);
			gl.compileShader(gl_shaders[idx]);
			break;
		case 'attachShader':
			gl.compileShader(gl_programs[uint(args[0])], gl_shaders[uint(args[1])]);
			break;
		case 'bufferData':
			gl.bufferData(uint(args[0]), args[2], uint(args[3]));
			break;

		default:
			// DOESNT WORK >_>
			eval('gl.' + cmd).apply(gl, args);
			break;
	}
};

/**
var socket_send = function(obj){
  var data = JSON.stringify(obj);
  socket.send(data);
};
**/

window.APP = {};
$(document).ready(function() {
    console.log("document ready");

    window.canvas = document.getElementById('canvas');
    window.addEventListener("resize", canvas_resize)
	window.gl = window.canvas.getContext("experimental-webgl");
	window.gl.viewportWidth = window.canvas.width;
	window.gl.viewportHeight = window.canvas.height;
    canvas_resize();
    console.log("canvas initialized...")

    var url = "ws://localhost:8888/";
    window.socket = new WebSocket(url, "glproxy");
    window.socket.binaryType = "arraybuffer";
    window.socket.onmessage = socket_recv;
    console.log("connected to websocket:", url);

	/**
    $('html').live("mousemove", function(e) {
        //console.log("mouse_move", e);
        socket_send({
          type: "mouse_move",
          x: e.pageX,
          y: e.pageY,
          which: e.which,
        });
        return false;
    });

    $('html').live("mousedown", function(e) {
        //console.log("mouse_down", e);
        socket_send({
          type: "mouse_down",
          x: e.pageX,
          y: e.pageY,
          which: e.which,
        });
        return false;
    });

    $('html').live("mouseup", function(e) {
        //console.log("mouse_up", e);
        socket_send({
          type: "mouse_up",
          x: e.pageX,
          y: e.pageY,
          which: e.which,
        });
        return false;
    });

    $('html').live("keydown", function(e) {
        //console.log("key_down", e);
        socket_send({
          type: "key_down",
          charCode: e.keyCode,
          keyCode: e.keyCode,
          shiftKey: e.shiftKey, 
          ctrlKey: e.ctrlKey, 
          metaKey: e.metaKey, 
          altKey: e.altKey, 
        });
        return false;
    });
	**/

});





