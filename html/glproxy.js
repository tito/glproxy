var canvas_resize = function(){
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
	console.log('canvas size is:' + window.innerWidth);
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
var gl_framebuffers = {}
var gl_framebuffer_idx = 1;
var gl_renderbuffers = {}
var gl_renderbuffer_idx = 1;
var gl_buffers = {}
var gl_buffer_idx = 1;
var gl_current_program;
var gl_symbols = [
"activeTexture",
"attachShader",
"bindAttribLocation",
"bindBuffer",
"bindFramebuffer",
"bindRenderbuffer",
"bindTexture",
"blendColor",
"blendEquation",
"blendEquationSeparate",
"blendFunc",
"blendFuncSeparate",
"bufferData",
"bufferSubData",
"checkFramebufferStatus",
"clear",
"clearColor",
"clearDepthf",
"clearStencil",
"colorMask",
"compileShader",
"compressedTexImage2D",
"compressedTexSubImage2D",
"copyTexImage2D",
"copyTexSubImage2D",
"createProgram",
"createShader",
"cullFace",
"deleteBuffers",
"deleteFramebuffers",
"deleteProgram",
"deleteRenderbuffers",
"deleteShader",
"deleteTextures",
"depthFunc",
"depthMask",
"depthRangef",
"detachShader",
"disable",
"disableVertexAttribArray",
"drawArrays",
"drawElements",
"enable",
"enableVertexAttribArray",
"finish",
"flush",
"framebufferRenderbuffer",
"framebufferTexture2D",
"frontFace",
"genBuffers",
"generateMipmap",
"genFramebuffers",
"genRenderbuffers",
"genTextures",
"getActiveAttrib",
"getActiveUniform",
"getAttachedShaders",
"getAttribLocation",
"getBooleanv",
"getBufferParameteriv",
"getError",
"getFloatv",
"getFramebufferAttachmentParameteriv",
"getIntegerv",
"getProgramiv",
"getProgramInfoLog",
"getRenderbufferParameteriv",
"getShaderiv",
"getShaderInfoLog",
"getShaderPrecisionFormat",
"getShaderSource",
"getString",
"getTexParameterfv",
"getTexParameteriv",
"getUniformfv",
"getUniformiv",
"getUniformLocation",
"getVertexAttribfv",
"getVertexAttribiv",
"getVertexAttribPointerv",
"hint",
"isBuffer",
"isEnabled",
"isFramebuffer",
"isProgram",
"isRenderbuffer",
"isShader",
"isTexture",
"lineWidth",
"linkProgram",
"pixelStorei",
"polygonOffset",
"readPixels",
"releaseShaderCompiler",
"renderbufferStorage",
"sampleCoverage",
"scissor",
"shaderBinary",
"shaderSource",
"stencilFunc",
"stencilFuncSeparate",
"stencilMask",
"stencilMaskSeparate",
"stencilOp",
"stencilOpSeparate",
"texImage2D",
"texParameterf",
"texParameterfv",
"texParameteri",
"texParameteriv",
"texSubImage2D",
"uniform1f",
"uniform1fv",
"uniform1i",
"uniform1iv",
"uniform2f",
"uniform2fv",
"uniform2i",
"uniform2iv",
"uniform3f",
"uniform3fv",
"uniform3i",
"uniform3iv",
"uniform4f",
"uniform4fv",
"uniform4i",
"uniform4iv",
"uniformMatrix2fv",
"uniformMatrix3fv",
"uniformMatrix4fv",
"useProgram",
"validateProgram",
"vertexAttrib1f",
"vertexAttrib1fv",
"vertexAttrib2f",
"vertexAttrib2fv",
"vertexAttrib3f",
"vertexAttrib3fv",
"vertexAttrib4f",
"vertexAttrib4fv",
"vertexAttribPointer",
"viewport"
];

var M_CLR = 256; /* clear table marker */
var M_EOD = 257; /* end-of-data marker */
var M_NEW = 258; /* new code index */
var frame = 0;
// Decompress an LZW-encoded string
function lzw_decode(bin)
{
	var out = [];

	function new_dec_t() {
		// prev, back, c
		var dt = [];
		for ( var i = 0; i < 512; i++ )
			dt.push([0, 0, 0]);
		return dt;
	}
 
	var d = new_dec_t();
	var len, j, next_shift = 512, bits = 9, n_bits = 0;
	var code, c, t, next_code = M_NEW;
 
	var tmp = 0;
	function get_code() {
		while(n_bits < bits) {
			if (len > 0) {
				len --;
				tmp = (tmp << 8) | bin[0];
				bin = bin.subarray(1);
				n_bits += 8;
			} else {
				tmp = tmp << (bits - n_bits);
				n_bits = bits;
			}
		}
		n_bits -= bits;
		code = tmp >> n_bits;
		tmp &= (1 << n_bits) - 1;
	}
 
	function clear_table() {
		d = new_dec_t();
		for (j = 0; j < 256; j++) d[j][2] = j;
		next_code = M_NEW;
		next_shift = 512;
		bits = 9;
	};

	function _setsize(d, ndl) {
		for (j = d.length; j < ndl; j++)
			d.push([0, 0, 0]);
	}
 
	clear_table(); /* in case encoded bits didn't start with M_CLR */
	for (len = bin.byteLength; len;) {
		get_code();
		if (code == M_EOD) break;
		if (code == M_CLR) {
			clear_table();
			continue;
		}
 
		if (code >= next_code) {
			console.error("Bad sequence");
			return;
		}
 
		d[next_code][0] = c = code;
		while (c > 255) {
			t = d[c][0]; d[t][1] = c; c = t;
		}
 
		d[next_code - 1][2] = c;
 
		while (d[c][1]) {
			out.push(d[c][2]);
			t = d[c][1]; d[c][1] = 0; c = t;
		}
		out.push(d[c][2]);
		if (++next_code >= next_shift) {
			if (++bits > 16) {
				/* if input was correct, we'd have hit M_CLR before this */
				console.error("Too many bits");
				return;
			}
			next_shift *= 2;
			_setsize(d, next_shift);
		}
	}
 
	/* might be ok, so just whine, don't be drastic */
	if (code != M_EOD)
		console.warn("Bits did not end in EOD");
 
	return new Uint8Array(out);
}

var z = [];
var socket_recv = function(msg){
	var buf = new Uint8Array(msg.data);
	buf = lzw_decode(buf);
	//console.log(buf);
	//console.log(cstr(buf));
	//return;

	var cmd = gl_symbols[uint(buf.subarray(0, 4))];
	buf = buf.subarray(4);
	data = buf;
	var iarg = uint(buf);
	buf = buf.subarray(4);

	//console.log(cmd);
	//return;

	var args = new Array();
	for ( i = 0; i < iarg; i++ ) {
		var size = uint(buf);
		buf = buf.subarray(4);
		args.push(buf.subarray(0, size));
		buf = buf.subarray(size);
	}

	if ( cmd == 'enable' && uint(args[0]) == 0x9999 )
		display();
	else {
		z.push([cmd, args]);
		if ( frame == 0 )
			showlog("Waiting a complete frame (" + z.length + ")");
	}
}

function display() {
	frame++;
	hidelog();
	//console.debug('>> show frame with ' + z.length + ' commands');
	for ( var i = 0; i < z.length; i++ ) {
		var cmd = z[i][0];
		var args = z[i][1];
		runcmd(cmd, args);
	}
	z = [];
}

function showlog(msg) {
	$('#log').show();
	$('#logmsg').html(msg);
}

function hidelog(msg) {
	$('#log').fadeOut();
}

function runcmd(cmd, args) {

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
			//console.log('bindAttribLocation');
			//console.log(args);
			//console.log(uint(args[0]));
			//console.log(uint(args[1]));
			//console.log(cstr(args[2]));
			//console.log(gl_programs[uint(args[0])]);
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
		case 'genRenderbuffers':
			gl_renderbuffers[gl_renderbuffer_idx] = gl.createFramebuffer();
			gl_renderbuffer_idx++;
			break;
		case 'genFramebuffers':
			gl_framebuffers[gl_framebuffer_idx] = gl.createFramebuffer();
			gl_framebuffer_idx++;
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
			gl.bindTexture(uint(args[0]), gl_textures[uint(args[1])]);
			break;
		case 'bindFramebuffer':
			gl.bindFramebuffer(uint(args[0]), gl_framebuffers[uint(args[1])]);
			break;
		case 'bindRenderbuffer':
			gl.bindRenderbuffer(uint(args[0]), gl_renderbuffers[uint(args[1])]);
			break;
		case 'framebufferTexture2D':
			gl.framebufferTexture2D(uint(args[0]), uint(args[1]), uint(args[2]), gl_textures[uint(args[3])], uint(args[4]));
			break;
		case 'framebufferRenderbuffer':
			gl.framebufferRenderbuffer(uint(args[0]), uint(args[1]), uint(args[3]), gl_renderbuffers[uint(args[4])]);
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
			//console.log('load buffer of ' + uint(args[1]), '->', args[2].byteLength);
			gl.bufferData(uint(args[0]), args[2], uint(args[3]));
			break;
		case 'bufferSubData':
			gl.bufferSubData(uint(args[0]), uint(args[1]), args[3]);
			break;
		case 'bindAttribLocation':
			gl.bindAttribLocation(uint(args[0]), uint(args[1]), cstr(args[2]));
			break;
		case 'shaderSource':
			//console.log('load shader: ' + cstr2(args[2]));
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
			//console.log(args);
			//console.log(cstr(args[0]));
			loc = gl.getUniformLocation(gl_current_program, cstr(args[0]));
			//console.log(gl_current_program);
			//console.log(loc);
			//console.log(cfloat32(args[2]));
			gl.uniformMatrix4fv(gl.getUniformLocation(gl_current_program, cstr(args[0])), false, cfloat32(args[2]));
			break;
		case 'texImage2D':
			//console.log('load image ' + uint(args[3]) + 'x' + uint(args[4]))
			gl.texImage2D(uint(args[0]), uint(args[1]), uint(args[2]),
				uint(args[3]), uint(args[4]), uint(args[5]), uint(args[6]),
				uint(args[7]), args[8]);
			break;
		case 'texSubImage2D':
			//console.log('load subimage ' + uint(args[4]) + 'x' + uint(args[5]))
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
			//console.log('draw ' + uint(args[1]) + ' elements, mode=' + uint(args[0]) + ', type=' + uint(args[2]));
			indices = ushortv(args[3]);
			//console.log(indices);
			//console.log('before bindBuffer:');
			var buffer = gl.createBuffer();
			gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buffer);
			gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.DYNAMIC_DRAW);
			//console.log('after bindBuffer:' + gl.getError());
			gl.drawElements(uint(args[0]), uint(args[1]), uint(args[2]), 0);
			//console.log('after drawElements:' + gl.getError());
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
		case 'colorMask':
			gl.colorMask(uint(args[0]), uint(args[1]), uint(args[2]), uint(args[3]));
			break;
		case 'stencilOp':
			gl.stencilOp(uint(args[0]), uint(args[1]), uint(args[2]));
			break;
		case 'stencilFunc':
			gl.stencilFunc(uint(args[0]), uint(args[1]), uint(args[2]));
			break;
		case 'stencilMask':
			gl.stencilMask(uint(args[0]));
			break;
		case 'clearStencil':
			gl.clearStencil(uint(args[0]));
			break;
		case 'deleteTextures':
			gl.deleteTexture(gl_textures[uint(args[0])]);
			break;
		case 'deleteBuffers':
			gl.deleteBuffer(gl_buffers[uint(args[0])]);
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
	/**
	else
		console.log('cmd=' + cmd);
	**/
};

var socket_send = function(obj){
	var data = new Uint16Array([obj.type, obj.x, obj.y, obj.which]);
	socket.send(data.buffer);
};

var socket_open = function(obj) {
	// at the connect, send the viewport size
	var data = new Uint16Array([99, gl.viewportWidth, gl.viewportHeight, 0]);
	socket.send(data.buffer);
	showlog("Waiting a complete frame");
	retry = 0;
	frame = 0;
}

function reset_gl() {
	// XXX FREE !!!!
	gl_textures = {}
	gl_texture_idx = 1;
	gl_shaders = {}
	gl_shader_idx = 1;
	gl_programs = {}
	gl_program_idx = 1;
	gl_framebuffers = {}
	gl_framebuffer_idx = 1;
	gl_renderbuffers = {}
	gl_renderbuffer_idx = 1;
	gl_buffers = {}
	gl_buffer_idx = 1;
	gl_current_program = 0;
	z = [];
}

var socket_close = function(obj) {
	console.log('Server disconnected');
	setTimeout("tryconnect()", 2000);
}

var socket_error = function(obj) {
	console.error('Server error:' + obj);
	setTimeout("tryconnect()", 2000);
}

var retry = 0;
function tryconnect() {
	reset_gl();
	var url = location.hash.substring(1);
	if ( url == "" )
		url = "localhost:8888";
    url = "ws://" + url + "/";
	showlog("Connecting to " + url + " <i>(" + retry + ")</i>");
	retry++;
    window.socket = new WebSocket(url, "glproxy");
    window.socket.binaryType = "arraybuffer";
    window.socket.onmessage = socket_recv;
	window.socket.onopen = socket_open;
	window.socket.onclose = socket_close;
	window.socket.onerror = socket_error;
    console.log("connecting to websocket...", url);
}

window.APP = {};
$(document).ready(function() {
    console.log("document ready");

    window.canvas = document.getElementById('canvas');
    window.addEventListener("resize", canvas_resize)
	window.gl = window.canvas.getContext("experimental-webgl");
    canvas_resize();
	window.gl.viewportWidth = window.canvas.width;
	window.gl.viewportHeight = window.canvas.height;
    console.log("canvas initialized...")
	tryconnect();


    $('html').live("mousemove", function(e) {
        //console.log("mouse_move", e);
        socket_send({
          type: 0,
          x: e.pageX,
          y: e.pageY,
          which: e.which,
        });
        return false;
    });

    $('html').live("mousedown", function(e) {
        //console.log("mouse_down", e);
        socket_send({
          type: 1,
          x: e.pageX,
          y: e.pageY,
          which: e.which,
        });
        return false;
    });

    $('html').live("mouseup", function(e) {
        //console.log("mouse_up", e);
        socket_send({
          type: 2,
          x: e.pageX,
          y: e.pageY,
          which: e.which,
        });
        return false;
    });

	/**
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





