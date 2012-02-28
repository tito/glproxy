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
	var l = 0;
	l += buf[0];
	l += buf[1] << 8;
	l += buf[2] << 16;
	l += buf[3] << 24;
	return l;
	//return new Uint32Array(buf.subarray(0, 4).buffer)[0];
}

function ushort(buf) {
	var l = 0;
	l += buf[0];
	l += buf[1] << 8;
	return l;
	//return new Uint16Array(buf.subarray(0, 2).buffer)[0];
}

function ushortv(buf) {
	buf = buf.buffer.slice(buf.byteOffset, buf.byteOffset + buf.byteLength);
	return new Uint16Array(buf);
}

function cfloat32(buf) {
	buf = buf.buffer.slice(buf.byteOffset, buf.byteOffset + buf.byteLength);
	return new Float32Array(buf);
}

function cstr(buf) {
	var cmd = '';
	for (var i = 0; i < buf.byteLength; i++)
		cmd += String.fromCharCode(buf[i])
    return cmd;
}

function cstr2(buf) {
	var cmd = '';
	for (var i = 0; i < buf.byteLength; i++) {
		if (buf[i] == 0)
			break;
		cmd += String.fromCharCode(buf[i])
	}
    return cmd;
}

var data;
var gl_textures = {}
var gl_texture_idx = 1;
var gl_shaders = {}
var gl_shader_idx = 1;
var gl_programs = {}
var gl_program_idx = 1;
var gl_buffers = {}
var gl_buffer_idx = 1;
var gl_current_program;
var socket_recv = function(msg){
	var buf = new Uint8Array(msg.data).subarray(20);
	var cmd = readCommand(buf);
	buf = buf.subarray(cmd.length + 1);
	data = buf;
	var iarg = uint(buf);
	buf = buf.subarray(4);

	var args = new Array();
	for ( i = 0; i < iarg; i++ ) {
		var size = uint(buf);
		buf = buf.subarray(4);
		args.push(buf.subarray(0, size));
		buf = buf.subarray(size);
	}

	//console.log(cmd);
	//console.log(args);

	// fix some calls
	switch (cmd) {
		case 'clear':
			gl.clear(uint(args[0]));
			break;
		case 'clearColor':
			gl.clearColor(cfloat32(args[0]), cfloat32(args[1]), cfloat32(args[2]), cfloat32(args[3]));
			break;
		case 'finish':
			gl.finish();
			break;
		case 'finish':
			gl.finish();
			break;
		case 'flush':
			gl.flush();
			break;
		case 'enable':
			gl.enable(uint(args[0]));
			break;
		case 'disable':
			gl.disable(uint(args[0]));
			break;
		case 'pixelStorei':
			gl.pixelStorei(uint(args[0]), uint(args[1]));
			break;
		case 'texParameteri':
			gl.texParameteri(uint(args[0]), uint(args[1]), uint(args[2]));
			break;
		case 'bindAttribLocation':
			console.log('bindAttribLocation');
			console.log(args);
			console.log(uint(args[0]));
			console.log(uint(args[1]));
			console.log(cstr(args[2]));
			console.log(gl_programs[uint(args[0])]);
			gl.bindAttribLocation(gl_programs[uint(args[0])], uint(args[1]), cstr(args[2]));
			break;
		case 'blendFunc':
			gl.blendFunc(uint(args[0]), uint(args[1]));
			break;
		case 'activeTexture':
			gl.activeTexture(uint(args[0]));
			break;
		case 'genTextures':
			gl_textures[gl_texture_idx] = gl.createTexture();
			gl_texture_idx++;
			break;
		case 'createShader':
			gl_shaders[gl_shader_idx] = gl.createShader(uint(args[0]));
			gl_shader_idx++;
			break;
		case 'createProgram':
			gl_programs[gl_program_idx] = gl.createProgram();
			gl_program_idx++;
			break;
		case 'genBuffers':
			gl_buffers[gl_buffer_idx] = gl.createBuffer();
			gl_buffer_idx++;
			break;
		case 'bindBuffer':
			idx = uint(args[1]);
			gl.bindBuffer(uint(args[0]), gl_buffers[idx]);
			break;
		case 'bindTexture':
			//console.log("gl.bindTexture(" + uint(args[0]) + ", " + gl_textures[uint(args[1])] + "(" + uint(args[1]) + "))");
			gl.bindTexture(uint(args[0]), gl_textures[uint(args[1])]);
			break;
		case 'useProgram':
			idx = uint(args[0]);
			if ( idx == 0 )
				break;
			gl_current_program = gl_programs[idx];
			gl.useProgram(gl_current_program);
			break;
		case 'compileShader':
			idx = uint(args[0]);
			shader = gl_shaders[idx];
			gl.compileShader(shader);
			if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS))
				console.log('WARNING: ' + gl.getShaderInfoLog(shader));
			break;
		case 'attachShader':
			gl.attachShader(gl_programs[uint(args[0])], gl_shaders[uint(args[1])]);
			break;
		case 'bufferData':
			console.log('load buffer of ' + uint(args[1]), '->', args[2].byteLength);
			gl.bufferData(uint(args[0]), args[2], uint(args[3]));
			break;
		case 'bufferSubData':
			gl.bufferData(uint(args[0]), uint(args[1]), args[3]);
			break;
		case 'bindAttribLocation':
			gl.bindAttribLocation(uint(args[0]), uint(args[1]), cstr(args[2]));
			break;
		case 'shaderSource':
			console.log('load shader: ' + cstr2(args[2]));
			shader = gl_shaders[uint(args[0])];
			shader.source = cstr2(args[2]);
			gl.shaderSource(shader, shader.source);
			break;
		case 'linkProgram':
			gl.linkProgram(gl_programs[uint(args[0])]);
			break;
		case 'uniform1f':
			gl.uniform1f(gl.getUniformLocation(gl_current_program, cstr(args[0])),
				cfloat32(args[1].subarray(0, 4))[0]
			);
			break;
		case 'uniform2f':
			gl.uniform2f(gl.getUniformLocation(gl_current_program, cstr(args[0])),
				cfloat32(args[1])[0],
				cfloat32(args[2])[0]
			);
			break;
		case 'uniform3f':
			gl.uniform3f(gl.getUniformLocation(gl_current_program, cstr(args[0])),
				cfloat32(args[1])[0],
				cfloat32(args[2])[0],
				cfloat32(args[3])[0]
			);
			break;
		case 'uniform4f':
			gl.uniform4f(gl.getUniformLocation(gl_current_program, cstr(args[0])),
				cfloat32(args[1])[0],
				cfloat32(args[2])[0],
				cfloat32(args[3])[0],
				cfloat32(args[4])[0]
			);
			break;
		case 'uniform1i':
			gl.uniform1i(gl.getUniformLocation(gl_current_program, cstr(args[0])),
				uint(args[1])
			);
			break;
		case 'uniform2i':
			gl.uniform2i(gl.getUniformLocation(gl_current_program, cstr(args[0])),
				uint(args[1]),
				uint(args[2])
			);
			break;
		case 'uniform3i':
			gl.uniform3i(gl.getUniformLocation(gl_current_program, cstr(args[0])),
				uint(args[1]),
				uint(args[2]),
				uint(args[3])
			);
			break;
		case 'uniform4i':
			gl.uniform4i(gl.getUniformLocation(gl_current_program, cstr(args[0])),
				uint(args[1]),
				uint(args[2]),
				uint(args[3]),
				uint(args[4])
			);
			break;
		case 'uniform1fv':
			gl.uniform1fv(gl.getUniformLocation(gl_current_program, cstr(args[0])), cfloat32(args[1]));
			break;
		case 'uniform2fv':
			gl.uniform2fv(gl.getUniformLocation(gl_current_program, cstr(args[0])), cfloat32(args[1]));
			break;
		case 'uniform3fv':
			gl.uniform3fv(gl.getUniformLocation(gl_current_program, cstr(args[0])), cfloat32(args[1]));
			break;
		case 'uniform4fv':
			gl.uniform4fv(gl.getUniformLocation(gl_current_program, cstr(args[0])), cfloat32(args[1]));
			break;
		case 'uniformMatrix4fv':
			console.log(args);
			console.log(cstr(args[0]));
			loc = gl.getUniformLocation(gl_current_program, cstr(args[0]));
			console.log(gl_current_program);
			console.log(loc);
			console.log(cfloat32(args[2]));
			gl.uniformMatrix4fv(gl.getUniformLocation(gl_current_program, cstr(args[0])), false, cfloat32(args[2]));
			break;
		case 'texImage2D':
			console.log('load image ' + uint(args[3]) + 'x' + uint(args[4]))
			gl.texImage2D(uint(args[0]), uint(args[1]), uint(args[2]),
				uint(args[3]), uint(args[4]), uint(args[5]), uint(args[6]),
				uint(args[7]), args[8]);
			break;
		case 'texSubImage2D':
			console.log('load subimage ' + uint(args[4]) + 'x' + uint(args[5]))
			gl.texSubImage2D(uint(args[0]), uint(args[1]), uint(args[2]),
				uint(args[3]), uint(args[4]), uint(args[5]), uint(args[6]),
				uint(args[7]), args[8]);
			break;
		case 'viewport':
			a = uint(args[0]);
			b = uint(args[1]);
			c = uint(args[2]);
			d = uint(args[3]);
			console.log('viewport set to', a, b, c, d);
			gl.viewport(a, b, c, d);
			break;
		case 'drawElements':
			console.log('draw ' + uint(args[1]) + ' elements, mode=' + uint(args[0]) + ', type=' + uint(args[2]));
			indices = ushortv(args[3]);
			console.log(indices);
			console.log('before bindBuffer:');
			var buffer = gl.createBuffer();
			gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buffer);
			gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.DYNAMIC_DRAW);
			console.log('after bindBuffer:' + gl.getError());
			gl.drawElements(uint(args[0]), uint(args[1]), uint(args[2]), 0);
			console.log('after drawElements:' + gl.getError());
			break;
		case 'vertexAttribPointer':
			//console.log('attrib pointer size=' + uint(args[1]) + ' ptr=' + uint(args[5]));
			//console.log(args)
			gl.vertexAttribPointer(uint(args[0]), uint(args[1]), uint(args[2]), uint(args[3]),
					uint(args[4]), uint(args[5]));
			break;
		case 'enableVertexAttribArray':
			gl.enableVertexAttribArray(uint(args[0]));
			break;
		default:
			// DOESNT WORK >_>
			//eval('gl.' + cmd).apply(gl, args);
			console.warn('missing call for ' + cmd);
			break;
	}

	var err = gl.getError();
	if (err)
		console.error('cmd=' + cmd + ', error=' + err);
	else
		console.log('cmd=' + cmd);
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





