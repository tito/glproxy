#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <libwebsockets.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

//-----------------------------------------------------------------------------
// QUEUES
//-----------------------------------------------------------------------------

int server_port = 8888;
pthread_mutex_t qfuncmtx = PTHREAD_MUTEX_INITIALIZER;

enum demo_protocols {
	PROTOCOL_GLPROXY,
	DEMO_PROTOCOL_COUNT
};

typedef struct s_func_args_t {
	int size;
	void *data;
	struct s_func_args_t *next;
} func_args_t;

typedef struct s_func_t {
	char name[64];
	int argc;
	func_args_t *args;
	func_args_t *argst;
	struct s_func_t *next;
} func_t;

func_t *qfunc = NULL;
func_t *qfunct = NULL;
unsigned int qlen = 0;
func_t *f_create(char *name) {
	func_t *func;
	func = malloc(sizeof(func_t));
	strcpy(func->name, name);
	func->argc = 0;
	func->next = NULL;
	func->args = NULL;
	func->argst = NULL;
	return func;
}

void f_append(func_t *func, unsigned int size, void *data) {
	func_args_t *arg;
	arg = malloc(sizeof(func_args_t));
	printf("==> push an arg %p (size=%u, data=%p)\n", arg, size, data);
	arg->size = size;
	arg->next = NULL;
	arg->data = NULL;

	if ( size > 0 ) {
		arg->data = malloc(size);
		memcpy(arg->data, data, size);
	}

	if ( func->argst ) {
		func->argst->next = arg;
		func->argst = arg;
	} else {
		func->args = func->argst = arg;
	}
	printf("==< push an arg %p\n", arg);
}

void f_push(func_t *func) {
	printf("==> PUSH %p\n", func);
	pthread_mutex_lock(&qfuncmtx);
	if ( qfunc == NULL ) {
		qfunc = qfunct = func;
	} else {
		qfunct->next = func;
		qfunct = func;
	}
	qlen ++;
	pthread_mutex_unlock(&qfuncmtx);
	printf("==< PUSH %p\n", func);
}

func_t *f_pop() {
	func_t *func;
	pthread_mutex_lock(&qfuncmtx);
	if ( qfunc == NULL ) {
		pthread_mutex_unlock(&qfuncmtx);
		return NULL;
	}
	func = qfunc;
	qfunc = qfunc->next;
	if ( qfunc == NULL )
		qfunct = NULL;
	qlen --;
	pthread_mutex_unlock(&qfuncmtx);
	return func;
}

void f_free(func_t *func) {
	func_args_t *tmparg, *arg = func->args;
	while (arg) {
		tmparg = arg->next;
		if (arg->data)
			free(arg->data);
		free(arg);
		arg = tmparg;
	}
	free(func);
}

//-----------------------------------------------------------------------------
// OPENGL REDIRECTION
//-----------------------------------------------------------------------------

#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_COMPILE_STATUS                 0x8B81
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_LINK_STATUS                    0x8B82

typedef void            GLvoid;
typedef char            GLchar;
typedef unsigned int    GLenum;
typedef unsigned char   GLboolean;
typedef unsigned int    GLbitfield;
typedef signed char     GLbyte;
typedef short           GLshort;
typedef int             GLint;
typedef int             GLsizei;
typedef unsigned char   GLubyte;
typedef unsigned short  GLushort;
typedef unsigned int    GLuint;
typedef float           GLfloat;
typedef float           GLclampf;
typedef int             GLfixed;
typedef int             GLclampx;

/* GL types for handling large vertex buffer objects */
typedef long            GLintptr;
typedef long            GLsizeiptr;

static unsigned int _gl_program = 1;
static unsigned int _gl_shader = 1;
static unsigned int _gl_buffers = 0;
static unsigned int _gl_framebuffers = 0;
static unsigned int _gl_renderbuffers = 0;
static unsigned int _gl_textures = 0;

void         glActiveTexture (GLenum texture) {
	printf("--> glActiveTexture ()\n");
	func_t *func = f_create("glActiveTexture");
	f_append(func, sizeof(GLenum), &texture);
	f_push(func);
}

void         glAttachShader (GLuint program, GLuint shader) {
	printf("--> glAttachShader ()\n");
	func_t *func = f_create("glAttachShader");
	f_append(func, sizeof(GLuint), &program);
	f_append(func, sizeof(GLuint), &shader);
	f_push(func);
}

void         glBindAttribLocation (GLuint program, GLuint index, const GLchar* name) {
	printf("--> glBindAttribLocation ()\n");
	func_t *func = f_create("glBindAttribLocation");
	f_append(func, sizeof(GLuint), &program);
	f_append(func, sizeof(GLuint), &index);
	f_append(func, strlen(name) + 1, name);
	f_push(func);
}

void         glBindBuffer (GLenum target, GLuint buffer) {
	printf("--> glBindBuffer ()\n");
	func_t *func = f_create("glBindBuffer");
	f_append(func, sizeof(GLenum), &target);
	f_append(func, sizeof(GLuint), &buffer);
	f_push(func);
}

void         glBindFramebuffer (GLenum target, GLuint framebuffer) {
	printf("--> glBindFramebuffer ()\n");
	func_t *func = f_create("glBindFramebuffer");
	f_append(func, sizeof(GLenum), &target);
	f_append(func, sizeof(GLuint), &framebuffer);
	f_push(func);
}

void         glBindRenderbuffer (GLenum target, GLuint renderbuffer) {
	printf("--> glBindRenderbuffer ()\n");
	func_t *func = f_create("glBindRenderbuffer");
	f_append(func, sizeof(GLenum), &target);
	f_append(func, sizeof(GLuint), &renderbuffer);
	f_push(func);
}

void         glBindTexture (GLenum target, GLuint texture) {
	printf("--> glBindTexture ()\n");
	func_t *func = f_create("glBindTexture");
	f_append(func, sizeof(GLenum), &target);
	f_append(func, sizeof(GLuint), &texture);
	f_push(func);
}

void         glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	printf("--> glBlendColor ()\n");
	func_t *func = f_create("glBlendColor");
	f_append(func, sizeof(GLclampf), &red);
	f_append(func, sizeof(GLclampf), &green);
	f_append(func, sizeof(GLclampf), &blue);
	f_append(func, sizeof(GLclampf), &alpha);
	f_push(func);
}

void         glBlendEquation ( GLenum mode ) {
	printf("--> glBlendEquation ()\n");
	func_t *func = f_create("glBlendColor");
	f_append(func, sizeof(GLenum), &mode);
	f_push(func);
}

void         glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha) {
	printf("--> glBlendEquationSeparate ()\n");
	func_t *func = f_create("glBlendEquationSeparate");
	f_append(func, sizeof(GLenum), &modeRGB);
	f_append(func, sizeof(GLenum), &modeAlpha);
	f_push(func);
}

void         glBlendFunc (GLenum sfactor, GLenum dfactor) {
	printf("--> glBlendFunc ()\n");
	func_t *func = f_create("glBlendFunc");
	f_append(func, sizeof(GLenum), &sfactor);
	f_append(func, sizeof(GLenum), &dfactor);
	f_push(func);
}

void         glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
	printf("--> glBlendFuncSeparate ()\n");
	func_t *func = f_create("glBlendFuncSeparate");
	f_append(func, sizeof(GLenum), &srcRGB);
	f_append(func, sizeof(GLenum), &dstRGB);
	f_append(func, sizeof(GLenum), &srcAlpha);
	f_append(func, sizeof(GLenum), &dstAlpha);
	f_push(func);
}

void         glBufferData (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) {
	printf("--> glBufferData ()\n");
	func_t *func = f_create("glBufferData");
	f_append(func, sizeof(GLenum), &target);
	f_append(func, sizeof(GLsizeiptr), &size);
	f_append(func, size, data);
	f_append(func, sizeof(GLenum), &usage);
	f_push(func);
}

void         glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) {
	printf("--> glBufferSubData ()\n");
	func_t *func = f_create("glBufferSubData");
	f_append(func, sizeof(GLenum), &target);
	f_append(func, sizeof(GLintptr), &offset);
	f_append(func, sizeof(GLsizeiptr), &size);
	f_append(func, size, data);
	f_push(func);
}

GLenum       glCheckFramebufferStatus (GLenum target) {
	printf("--> glCheckFramebufferStatus ()\n");
	// TODO
	return 0;
}

void         glClear (GLbitfield mask) {
	printf("--> glClear ()\n");
	func_t *func = f_create("glClear");
	f_append(func, sizeof(GLbitfield), &mask);
	f_push(func);
}

void         glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	printf("--> glClearColor ()\n");
	func_t *func = f_create("glClearColor");
	f_append(func, sizeof(GLclampf), &red);
	f_append(func, sizeof(GLclampf), &green);
	f_append(func, sizeof(GLclampf), &blue);
	f_append(func, sizeof(GLclampf), &alpha);
	f_push(func);
}

void         glClearDepthf (GLclampf depth) {
	printf("--> glClearDepthf ()\n");
	func_t *func = f_create("glClearDepthf");
	f_append(func, sizeof(GLclampf), &depth);
	f_push(func);
}

void         glClearStencil (GLint s) {
	printf("--> glClearStencil ()\n");
	func_t *func = f_create("glClearStencil");
	f_append(func, sizeof(GLint), &s);
	f_push(func);
}

void         glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	printf("--> glColorMask ()\n");
	func_t *func = f_create("glColorMask");
	f_append(func, sizeof(GLclampf), &red);
	f_append(func, sizeof(GLclampf), &green);
	f_append(func, sizeof(GLclampf), &blue);
	f_append(func, sizeof(GLclampf), &alpha);
	f_push(func);
}

void         glCompileShader (GLuint shader) {
	printf("--> glCompileShader ()\n");
	func_t *func = f_create("glCompileShader");
	f_append(func, sizeof(GLuint), &shader);
	f_push(func);
}

void         glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data) {
	printf("--> glCompressedTexImage2D ()\n");
	assert(0);
}

void         glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data) {
	printf("--> glCompressedTexSubImage2D ()\n");
	assert(0);
}

void         glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
	printf("--> glCopyTexImage2D ()\n");
	assert(0);
}

void         glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
	printf("--> glCopyTexSubImage2D ()\n");
	assert(0);
}

GLuint       glCreateProgram (void) {
	printf("--> glCreateProgram ()\n");
	func_t *func = f_create("glCreateProgram");
	f_push(func);
	return _gl_program++;
}

GLuint       glCreateShader (GLenum type) {
	printf("--> glCreateShader ()\n");
	func_t *func = f_create("glCreateShader");
	f_append(func, sizeof(type), &type);
	f_push(func);
	return _gl_shader++;
}

void         glCullFace (GLenum mode) {
	printf("--> glCullFace ()\n");
	assert(0);
}

void         glDeleteBuffers (GLsizei n, const GLuint* buffers) {
	printf("--> glDeleteBuffers ()\n");
}

void         glDeleteFramebuffers (GLsizei n, const GLuint* framebuffers) {
	printf("--> glDeleteFramebuffers ()\n");
}

void         glDeleteProgram (GLuint program) {
	printf("--> glDeleteProgram ()\n");
}

void         glDeleteRenderbuffers (GLsizei n, const GLuint* renderbuffers) {
	printf("--> glDeleteRenderbuffers ()\n");
}

void         glDeleteShader (GLuint shader) {
	printf("--> glDeleteShader ()\n");
}

void         glDeleteTextures (GLsizei n, const GLuint* textures) {
	printf("--> glDeleteTextures ()\n");
}

void         glDepthFunc (GLenum func) {
	printf("--> glDepthFunc ()\n");
}

void         glDepthMask (GLboolean flag) {
	printf("--> glDepthMask ()\n");
}

void         glDepthRangef (GLclampf zNear, GLclampf zFar) {
	printf("--> glDepthRangef ()\n");
}

void         glDetachShader (GLuint program, GLuint shader) {
	printf("--> glDetachShader ()\n");
}

void         glDisable (GLenum cap) {
	printf("--> glDisable ()\n");
	func_t *func = f_create("glDisable");
	f_append(func, sizeof(GLenum), &cap);
	f_push(func);
}

void         glDisableVertexAttribArray (GLuint index) {
	printf("--> glDisableVertexAttribArray ()\n");
	func_t *func = f_create("glDisableVertexArray");
	f_append(func, sizeof(GLuint), &index);
	f_push(func);
}

void         glDrawArrays (GLenum mode, GLint first, GLsizei count) {
	printf("--> glDrawArrays ()\n");
	func_t *func = f_create("glDrawArrays");
	f_append(func, sizeof(GLenum), &mode);
	f_append(func, sizeof(GLint), &first);
	f_append(func, sizeof(GLsizei), &count);
	f_push(func);
}

void         glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices) {
	printf("--> glDrawElements ()\n");
}

void         glEnable (GLenum cap) {
	printf("--> glEnable ()\n");
	func_t *func = f_create("glEnable");
	f_append(func, sizeof(GLenum), &cap);
	f_push(func);
}

void         glEnableVertexAttribArray (GLuint index) {
	printf("--> glEnableVertexAttribArray ()\n");
}

void         glFinish (void) {
	printf("--> glFinish ()\n");
	func_t *func = f_create("gFinish");
	f_push(func);
}

void         glFlush (void) {
	printf("--> glFlush ()\n");
	func_t *func = f_create("glFlush");
	f_push(func);
}

void         glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
	printf("--> glFramebufferRenderbuffer ()\n");
}

void         glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
	printf("--> glFramebufferTexture2D ()\n");
}

void         glFrontFace (GLenum mode) {
	printf("--> glFrontFace ()\n");
}

void         glGenBuffers (GLsizei n, GLuint* buffers) {
	printf("--> glGenBuffers ()\n");
	int i;
	for (i = 0; i < n; i++)
		buffers[i] = _gl_buffers + i;
	_gl_buffers += n;
}

void         glGenerateMipmap (GLenum target) {
	printf("--> glGenerateMipmap ()\n");
}

void         glGenFramebuffers (GLsizei n, GLuint* framebuffers) {
	printf("--> glGenFramebuffers ()\n");
	int i;
	for (i = 0; i < n; i++)
		framebuffers[i] = _gl_framebuffers + i;
	_gl_framebuffers += n;
}

void         glGenRenderbuffers (GLsizei n, GLuint* renderbuffers) {
	printf("--> glGenRenderbuffers ()\n");
	int i;
	for (i = 0; i < n; i++)
		renderbuffers[i] = _gl_renderbuffers + i;
	_gl_renderbuffers += n;
}

void         glGenTextures (GLsizei n, GLuint* textures) {
	printf("--> glGenTextures ()\n");
	int i;
	for (i = 0; i < n; i++)
		textures[i] = _gl_textures + i;
	_gl_textures += n;
}

void         glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
	printf("--> glGetActiveAttrib ()\n");
}

void         glGetActiveUniform (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
	printf("--> glGetActiveUniform ()\n");
}

void         glGetAttachedShaders (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders) {
	printf("--> glGetAttachedShaders ()\n");
}

int          glGetAttribLocation (GLuint program, const GLchar* name) {
	printf("--> glGetAttribLocation ()\n");
}

void         glGetBooleanv (GLenum pname, GLboolean* params) {
	printf("--> glGetBooleanv ()\n");
}

void         glGetBufferParameteriv (GLenum target, GLenum pname, GLint* params) {
	printf("--> glGetBufferParameteriv ()\n");
}

GLenum       glGetError (void) {
	printf("--> glGetError ()\n");
	return 0;
}

void         glGetFloatv (GLenum pname, GLfloat* params) {
	printf("--> glGetFloatv ()\n");
}

void         glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint* params) {
	printf("--> glGetFramebufferAttachmentParameteriv ()\n");
}

void         glGetIntegerv (GLenum pname, GLint* params) {
	printf("--> glGetIntegerv ()\n");
	*params = NULL;
	if ( pname == GL_MAX_TEXTURE_SIZE )
		*params = 4096;
	else if ( pname == GL_MAX_TEXTURE_IMAGE_UNITS )
		*params = 8;
}

void         glGetProgramiv (GLuint program, GLenum pname, GLint* params) {
	printf("--> glGetProgramiv ()\n");
	if (pname == GL_LINK_STATUS)
		*params = 1;
}

void         glGetProgramInfoLog (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog) {
	printf("--> glGetProgramInfoLog ()\n");
}

void         glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint* params) {
	printf("--> glGetRenderbufferParameteriv ()\n");
}

void         glGetShaderiv (GLuint shader, GLenum pname, GLint* params) {
	printf("--> glGetShaderiv ()\n");
	if (pname == GL_COMPILE_STATUS)
		*params = 1;
}

void         glGetShaderInfoLog (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog) {
	printf("--> glGetShaderInfoLog ()\n");
}

void         glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) {
	printf("--> glGetShaderPrecisionFormat ()\n");
}

void         glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source) {
	printf("--> glGetShaderSource ()\n");
}

const char* glGetString (GLenum name) {
	printf("--> glGetString()\n");
	static char *empty = "";
	static char *version = "OpenGL ES 2.0";
	static char *vendor = "Kivy team";
	static char *renderer = "GL proxy to websocket";
	static char *shadingversion = "GLSL 1.0";

	if ( name == GL_VERSION )
		return version;
	else if (name == GL_VENDOR )
		return vendor;
	else if (name == GL_RENDERER )
		return renderer;
	else if (name == GL_SHADING_LANGUAGE_VERSION)
		return shadingversion;

	return empty;
}

void         glGetTexParameterfv (GLenum target, GLenum pname, GLfloat* params) {
	printf("--> glGetTexParameterfv ()\n");
}

void         glGetTexParameteriv (GLenum target, GLenum pname, GLint* params) {
	printf("--> glGetTexParameteriv ()\n");
}

void         glGetUniformfv (GLuint program, GLint location, GLfloat* params) {
	printf("--> glGetUniformfv ()\n");
}

void         glGetUniformiv (GLuint program, GLint location, GLint* params) {
	printf("--> glGetUniformiv ()\n");
}

int          glGetUniformLocation (GLuint program, const GLchar* name) {
	printf("--> glGetUniformLocation ()\n");
}

void         glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat* params) {
	printf("--> glGetVertexAttribfv ()\n");
}

void         glGetVertexAttribiv (GLuint index, GLenum pname, GLint* params) {
	printf("--> glGetVertexAttribiv ()\n");
}

void         glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid** pointer) {
	printf("--> glGetVertexAttribPointerv ()\n");
}

void         glHint (GLenum target, GLenum mode) {
	printf("--> glHint ()\n");
}

GLboolean    glIsBuffer (GLuint buffer) {
	printf("--> glIsBuffer ()\n");
}

GLboolean    glIsEnabled (GLenum cap) {
	printf("--> glIsEnabled ()\n");
}

GLboolean    glIsFramebuffer (GLuint framebuffer) {
	printf("--> glIsFramebuffer ()\n");
}

GLboolean    glIsProgram (GLuint program) {
	printf("--> glIsProgram ()\n");
}

GLboolean    glIsRenderbuffer (GLuint renderbuffer) {
	printf("--> glIsRenderbuffer ()\n");
}

GLboolean    glIsShader (GLuint shader) {
	printf("--> glIsShader ()\n");
}

GLboolean    glIsTexture (GLuint texture) {
	printf("--> glIsTexture ()\n");
}

void         glLineWidth (GLfloat width) {
	printf("--> glLineWidth ()\n");
}

void         glLinkProgram (GLuint program) {
	printf("--> glLinkProgram ()\n");
}

void         glPixelStorei (GLenum pname, GLint param) {
	printf("--> glPixelStorei ()\n");
}

void         glPolygonOffset (GLfloat factor, GLfloat units) {
	printf("--> glPolygonOffset ()\n");
}

void         glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) {
	printf("--> glReadPixels ()\n");
}

void         glReleaseShaderCompiler (void) {
	printf("--> glReleaseShaderCompiler ()\n");
}

void         glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
	printf("--> glRenderbufferStorage ()\n");
}

void         glSampleCoverage (GLclampf value, GLboolean invert) {
	printf("--> glSampleCoverage ()\n");
}

void         glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
	printf("--> glScissor ()\n");
}

void         glShaderBinary (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length) {
	printf("--> glShaderBinary ()\n");
}

void         glShaderSource (GLuint shader, GLsizei count, const GLchar** string, const GLint* length) {
	printf("--> glShaderSource ()\n");
}

void         glStencilFunc (GLenum func, GLint ref, GLuint mask) {
	printf("--> glStencilFunc ()\n");
}

void         glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) {
	printf("--> glStencilFuncSeparate ()\n");
}

void         glStencilMask (GLuint mask) {
	printf("--> glStencilMask ()\n");
}

void         glStencilMaskSeparate (GLenum face, GLuint mask) {
	printf("--> glStencilMaskSeparate ()\n");
}

void         glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) {
	printf("--> glStencilOp ()\n");
}

void         glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass) {
	printf("--> glStencilOpSeparate ()\n");
}

void         glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
	printf("--> glTexImage2D ()\n");
}

void         glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
	printf("--> glTexParameterf ()\n");
}

void         glTexParameterfv (GLenum target, GLenum pname, const GLfloat* params) {
	printf("--> glTexParameterfv ()\n");
}

void         glTexParameteri (GLenum target, GLenum pname, GLint param) {
	printf("--> glTexParameteri ()\n");
}

void         glTexParameteriv (GLenum target, GLenum pname, const GLint* params) {
	printf("--> glTexParameteriv ()\n");
}

void         glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels) {
	printf("--> glTexSubImage2D ()\n");
}

void         glUniform1f (GLint location, GLfloat x) {
	printf("--> glUniform1f ()\n");
}

void         glUniform1fv (GLint location, GLsizei count, const GLfloat* v) {
	printf("--> glUniform1fv ()\n");
}

void         glUniform1i (GLint location, GLint x) {
	printf("--> glUniform1i ()\n");
}

void         glUniform1iv (GLint location, GLsizei count, const GLint* v) {
	printf("--> glUniform1iv ()\n");
}

void         glUniform2f (GLint location, GLfloat x, GLfloat y) {
	printf("--> glUniform2f ()\n");
}

void         glUniform2fv (GLint location, GLsizei count, const GLfloat* v) {
	printf("--> glUniform2fv ()\n");
}

void         glUniform2i (GLint location, GLint x, GLint y) {
	printf("--> glUniform2i ()\n");
}

void         glUniform2iv (GLint location, GLsizei count, const GLint* v) {
	printf("--> glUniform2iv ()\n");
}

void         glUniform3f (GLint location, GLfloat x, GLfloat y, GLfloat z) {
	printf("--> glUniform3f ()\n");
}

void         glUniform3fv (GLint location, GLsizei count, const GLfloat* v) {
	printf("--> glUniform3fv ()\n");
}

void         glUniform3i (GLint location, GLint x, GLint y, GLint z) {
	printf("--> glUniform3i ()\n");
}

void         glUniform3iv (GLint location, GLsizei count, const GLint* v) {
	printf("--> glUniform3iv ()\n");
}

void         glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	printf("--> glUniform4f ()\n");
}

void         glUniform4fv (GLint location, GLsizei count, const GLfloat* v) {
	printf("--> glUniform4fv ()\n");
}

void         glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w) {
	printf("--> glUniform4i ()\n");
}

void         glUniform4iv (GLint location, GLsizei count, const GLint* v) {
	printf("--> glUniform4iv ()\n");
}

void         glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
	printf("--> glUniformMatrix2fv ()\n");
}

void         glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
	printf("--> glUniformMatrix3fv ()\n");
}

void         glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
	printf("--> glUniformMatrix4fv ()\n");
}

void         glUseProgram (GLuint program) {
	printf("--> glUseProgram ()\n");
}

void         glValidateProgram (GLuint program) {
	printf("--> glValidateProgram ()\n");
}

void         glVertexAttrib1f (GLuint indx, GLfloat x) {
	printf("--> glVertexAttrib1f ()\n");
}

void         glVertexAttrib1fv (GLuint indx, const GLfloat* values) {
	printf("--> glVertexAttrib1fv ()\n");
}

void         glVertexAttrib2f (GLuint indx, GLfloat x, GLfloat y) {
	printf("--> glVertexAttrib2f ()\n");
}

void         glVertexAttrib2fv (GLuint indx, const GLfloat* values) {
	printf("--> glVertexAttrib2fv ()\n");
}

void         glVertexAttrib3f (GLuint indx, GLfloat x, GLfloat y, GLfloat z) {
	printf("--> glVertexAttrib3f ()\n");
}

void         glVertexAttrib3fv (GLuint indx, const GLfloat* values) {
	printf("--> glVertexAttrib3fv ()\n");
}

void         glVertexAttrib4f (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	printf("--> glVertexAttrib4f ()\n");
}

void         glVertexAttrib4fv (GLuint indx, const GLfloat* values) {
	printf("--> glVertexAttrib4fv ()\n");
}

void         glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) {
	printf("--> glVertexAttribPointer ()\n");
}

void         glViewport (GLint x, GLint y, GLsizei width, GLsizei height) {
	printf("--> glViewport ()\n");
}

#ifdef __APPLE__
#define IOSurfaceRef void *
#define CGLContextObj void *
#define CGLPixelFormatObj void *
#define CGLRendererInfoObj void *
#define CGLPBufferObj void *
#define CGLShareGroupObj void *
#define CGSConnectionID void *

#define CGLError int
#define CGLPixelFormatAttribute int
#define CGLRendererProperty int
#define CGLContextEnable int
#define CGLContextParameter int
#define CGLGlobalOption int
#define CGSWindowID int
#define CGSSurfaceID int

CGLError CGLSetCurrentContext(CGLContextObj ctx) {
	printf(" --> CGLError CGLSetCurrentContext(CGLContextObj ctx)\n");
	return 0;
}
CGLContextObj CGLGetCurrentContext(void) {
	printf(" --> CGLContextObj CGLGetCurrentContext(void)\n");
}
CGLError CGLChoosePixelFormat(const CGLPixelFormatAttribute * attribs, CGLPixelFormatObj * pix, GLint * npix) {
	printf(" --> CGLError CGLChoosePixelFormat(const CGLPixelFormatAttribute * attribs, CGLPixelFormatObj * pix, GLint * npix)\n");
	return 0;
}
CGLError CGLDestroyPixelFormat(CGLPixelFormatObj pix) {
	printf(" --> CGLError CGLDestroyPixelFormat(CGLPixelFormatObj pix)\n");
	return 0;
}
CGLError CGLDescribePixelFormat(CGLPixelFormatObj pix, GLint pix_num, CGLPixelFormatAttribute attrib, GLint * value) {
	printf(" --> CGLError CGLDescribePixelFormat(CGLPixelFormatObj pix, GLint pix_num, CGLPixelFormatAttribute attrib, GLint * value)\n");
	return 0;
}
void CGLReleasePixelFormat(CGLPixelFormatObj pix) {
	printf(" --> void CGLReleasePixelFormat(CGLPixelFormatObj pix)\n");
}
CGLPixelFormatObj CGLRetainPixelFormat(CGLPixelFormatObj pix) {
	printf(" --> CGLPixelFormatObj CGLRetainPixelFormat(CGLPixelFormatObj pix)\n");
}
GLuint CGLGetPixelFormatRetainCount(CGLPixelFormatObj pix) {
	printf(" --> GLuint CGLGetPixelFormatRetainCount(CGLPixelFormatObj pix)\n");
}
CGLError CGLQueryRendererInfo(GLuint display_mask, CGLRendererInfoObj * rend, GLint * nrend) {
	printf(" --> CGLError CGLQueryRendererInfo(GLuint display_mask, CGLRendererInfoObj * rend, GLint * nrend)\n");
}
CGLError CGLDestroyRendererInfo(CGLRendererInfoObj rend) {
	printf(" --> CGLError CGLDestroyRendererInfo(CGLRendererInfoObj rend)\n");
}
CGLError CGLDescribeRenderer(CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint * value) {
	printf(" --> CGLError CGLDescribeRenderer(CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint * value)\n");
}
CGLError CGLCreateContext(CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj * ctx) {
	printf(" --> CGLError CGLCreateContext(CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj * ctx)\n");
}
CGLError CGLDestroyContext(CGLContextObj ctx) {
	printf(" --> CGLError CGLDestroyContext(CGLContextObj ctx)\n");
}
CGLError CGLCopyContext(CGLContextObj src, CGLContextObj dst, GLbitfield mask) {
	printf(" --> CGLError CGLCopyContext(CGLContextObj src, CGLContextObj dst, GLbitfield mask)\n");
}
CGLContextObj CGLRetainContext(CGLContextObj ctx) {
	printf(" --> CGLContextObj CGLRetainContext(CGLContextObj ctx)\n");
}
void CGLReleaseContext(CGLContextObj ctx) {
	printf(" --> void CGLReleaseContext(CGLContextObj ctx)\n");
}
GLuint CGLGetContextRetainCount(CGLContextObj ctx) {
	printf(" --> GLuint CGLGetContextRetainCount(CGLContextObj ctx)\n");
}
CGLPixelFormatObj CGLGetPixelFormat(CGLContextObj ctx) {
	printf(" --> CGLPixelFormatObj CGLGetPixelFormat(CGLContextObj ctx)\n");
}
CGLError CGLCreatePBuffer(GLsizei width, GLsizei height, GLenum target, GLenum internalFormat, GLint max_level, CGLPBufferObj * pbuffer) {
	printf(" --> CGLError CGLCreatePBuffer(GLsizei width, GLsizei height, GLenum target, GLenum internalFormat, GLint max_level, CGLPBufferObj * pbuffer)\n");
}
CGLError CGLDestroyPBuffer(CGLPBufferObj pbuffer) {
	printf(" --> CGLError CGLDestroyPBuffer(CGLPBufferObj pbuffer)\n");
}
CGLError CGLDescribePBuffer(CGLPBufferObj obj, GLsizei * width, GLsizei * height, GLenum * target, GLenum * internalFormat, GLint * mipmap) {
	printf(" --> CGLError CGLDescribePBuffer(CGLPBufferObj obj, GLsizei * width, GLsizei * height, GLenum * target, GLenum * internalFormat, GLint * mipmap)\n");
}
CGLError CGLTexImagePBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum source) {
	printf(" --> CGLError CGLTexImagePBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum source)\n");
}
CGLPBufferObj CGLRetainPBuffer(CGLPBufferObj pbuffer) {
	printf(" --> CGLPBufferObj CGLRetainPBuffer(CGLPBufferObj pbuffer)\n");
}
void CGLReleasePBuffer(CGLPBufferObj pbuffer) {
	printf(" --> void CGLReleasePBuffer(CGLPBufferObj pbuffer)\n");
}
GLuint CGLGetPBufferRetainCount(CGLPBufferObj pbuffer) {
	printf(" --> GLuint CGLGetPBufferRetainCount(CGLPBufferObj pbuffer)\n");
}
CGLError CGLSetOffScreen(CGLContextObj ctx, GLsizei width, GLsizei height, GLint rowbytes, void * baseaddr) {
	printf(" --> CGLError CGLSetOffScreen(CGLContextObj ctx, GLsizei width, GLsizei height, GLint rowbytes, void * baseaddr)\n");
}
CGLError CGLGetOffScreen(CGLContextObj ctx, GLsizei * width, GLsizei * height, GLint * rowbytes, void * * baseaddr) {
	printf(" --> CGLError CGLGetOffScreen(CGLContextObj ctx, GLsizei * width, GLsizei * height, GLint * rowbytes, void * * baseaddr)\n");
}
CGLError CGLSetFullScreen(CGLContextObj ctx) {
	printf(" --> CGLError CGLSetFullScreen(CGLContextObj ctx)\n");
}
CGLError CGLSetFullScreenOnDisplay(CGLContextObj ctx, GLuint display_mask) {
	printf(" --> CGLError CGLSetFullScreenOnDisplay(CGLContextObj ctx, GLuint display_mask)\n");
}
CGLError CGLSetPBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum face, GLint level, GLint screen) {
	printf(" --> CGLError CGLSetPBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum face, GLint level, GLint screen)\n");
}
CGLError CGLGetPBuffer(CGLContextObj ctx, CGLPBufferObj * pbuffer, GLenum * face, GLint * level, GLint * screen) {
	printf(" --> CGLError CGLGetPBuffer(CGLContextObj ctx, CGLPBufferObj * pbuffer, GLenum * face, GLint * level, GLint * screen)\n");
}
CGLError CGLClearDrawable(CGLContextObj ctx) {
	printf(" --> CGLError CGLClearDrawable(CGLContextObj ctx)\n");
}
CGLError CGLFlushDrawable(CGLContextObj ctx) {
	printf(" --> CGLError CGLFlushDrawable(CGLContextObj ctx)\n");
}
CGLError CGLEnable(CGLContextObj ctx, CGLContextEnable pname) {
	printf(" --> CGLError CGLEnable(CGLContextObj ctx, CGLContextEnable pname)\n");
}
CGLError CGLDisable(CGLContextObj ctx, CGLContextEnable pname) {
	printf(" --> CGLError CGLDisable(CGLContextObj ctx, CGLContextEnable pname)\n");
}
CGLError CGLIsEnabled(CGLContextObj ctx, CGLContextEnable pname, GLint * enable) {
	printf(" --> CGLError CGLIsEnabled(CGLContextObj ctx, CGLContextEnable pname, GLint * enable)\n");
}
CGLError CGLSetParameter(CGLContextObj ctx, CGLContextParameter pname, const GLint * params) {
	printf(" --> CGLError CGLSetParameter(CGLContextObj ctx, CGLContextParameter pname, const GLint * params)\n");
}
CGLError CGLGetParameter(CGLContextObj ctx, CGLContextParameter pname, GLint * params) {
	printf(" --> CGLError CGLGetParameter(CGLContextObj ctx, CGLContextParameter pname, GLint * params)\n");
}
CGLError CGLSetVirtualScreen(CGLContextObj ctx, GLint screen) {
	printf(" --> CGLError CGLSetVirtualScreen(CGLContextObj ctx, GLint screen)\n");
}
CGLError CGLGetVirtualScreen(CGLContextObj ctx, GLint * screen) {
	printf(" --> CGLError CGLGetVirtualScreen(CGLContextObj ctx, GLint * screen)\n");
}
CGLError CGLSetGlobalOption(CGLGlobalOption pname, const GLint * params) {
	printf(" --> CGLError CGLSetGlobalOption(CGLGlobalOption pname, const GLint * params)\n");
}
CGLError CGLGetGlobalOption(CGLGlobalOption pname, GLint * params) {
	printf(" --> CGLError CGLGetGlobalOption(CGLGlobalOption pname, GLint * params)\n");
}
CGLError CGLSetOption(CGLGlobalOption pname, GLint param) {
	printf(" --> CGLError CGLSetOption(CGLGlobalOption pname, GLint param)\n");
}
CGLError CGLGetOption(CGLGlobalOption pname, GLint * param) {
	printf(" --> CGLError CGLGetOption(CGLGlobalOption pname, GLint * param)\n");
}
CGLError CGLLockContext(CGLContextObj ctx) {
	printf(" --> CGLError CGLLockContext(CGLContextObj ctx)\n");
}
CGLError CGLUnlockContext(CGLContextObj ctx) {
	printf(" --> CGLError CGLUnlockContext(CGLContextObj ctx)\n");
}
void CGLGetVersion(GLint * majorvers, GLint * minorvers) {
	printf(" --> void CGLGetVersion(GLint * majorvers, GLint * minorvers)\n");
}
const char * CGLErrorString(CGLError error) {
	printf(" --> const char * CGLErrorString(CGLError error)\n");
}
CGLError CGLTexImageIOSurface2D(CGLContextObj ctx, GLenum target, GLenum internal_format, GLsizei width, GLsizei height, GLenum format, GLenum type, IOSurfaceRef ioSurface, GLuint plane) {
	printf(" --> CGLError CGLTexImageIOSurface2D(CGLContextObj ctx, GLenum target, GLenum internal_format, GLsizei width, GLsizei height, GLenum format, GLenum type, IOSurfaceRef ioSurface, GLuint plane)\n");
}
CGLShareGroupObj CGLGetShareGroup(CGLContextObj ctx) {
	printf(" --> CGLShareGroupObj CGLGetShareGroup(CGLContextObj ctx)\n");
}
CGLError CGLSetSurface(CGLContextObj ctx, CGSConnectionID cid, CGSWindowID wid, CGSSurfaceID sid) {
	printf(" --> CGLError CGLSetSurface(CGLContextObj ctx, CGSConnectionID cid, CGSWindowID wid, CGSSurfaceID sid)\n");
}
CGLError CGLGetSurface(CGLContextObj ctx, CGSConnectionID * cid, CGSWindowID * wid, CGSSurfaceID * sid) {
	printf(" --> CGLError CGLGetSurface(CGLContextObj ctx, CGSConnectionID * cid, CGSWindowID * wid, CGSSurfaceID * sid)\n");
}
CGLError CGLUpdateContext(CGLContextObj ctx) {
	printf(" --> CGLError CGLUpdateContext(CGLContextObj ctx)\n");
}
CGLError CGLOpenCLMuxLockDown(void) {
	printf(" --> CGLError CGLOpenCLMuxLockDown(void)\n");
}

// others guess by run/error
void glFlushRenderAPPLE() { }
void glCheckFramebufferStatusEXT() { }
void glEnableClientState() { }
void glLoadIdentity() { }
void glMatrixMode() { }
void glOrtho() { }
void glTexCoordPointer() { }
void glTexEnvf() { }
void glVertexPointer() { }
#endif

//-----------------------------------------------------------------------------
// WEBSOCKET
//-----------------------------------------------------------------------------
static void hexdump(void *data, int size)
{
    /* dumps size bytes of *data to stdout. Looks like:
     * [0000] 75 6E 6B 6E 6F 77 6E 20
     *                  30 FF 00 00 00 00 39 00 unknown 0.....9.
     * (in a single line of course)
     */

    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }
            
        c = *p;
        if (isalnum(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) { 
            /* line completed */
            printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}


static int
callback_glproxy(
	struct libwebsocket_context * this,
	struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason,
	void *user, void *in, size_t len)
{
	struct per_session_data__glproxy *psd = user;
	func_t *func;
	func_args_t *arg;
	unsigned char *buf, *abuf;
	int n, size, argc = 0;

	printf("XXX callback glproxy %d\n", reason);

	switch (reason) {
		case LWS_CALLBACK_ESTABLISHED:
			break;
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			break;

		case LWS_CALLBACK_RECEIVE:
			printf("--> (%d) \"%.*s\"\n", (int)len, (int)len, (char *)in);
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
			// pop the queue, and send message!
			
			while (1) {
				printf("==> pop a function\n");
				func = f_pop();
				printf("==> poped %x\n", func);
				if (func == NULL)
					break;

				printf("==> %s state 1\n", func->name);
				// pass to calculate the size of the final buffer
				size = LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING;
				// name
				size += strlen(func->name) + 1;
				// number of args
				size += sizeof(int);
				arg = func->args;
				argc = 0;
				while ( arg ) {
					// size of the arg
					size += sizeof(int);
					size += arg->size;
					arg = arg->next;
					argc += 1;
				}

				printf("==> %s state 2, size is %d\n", func->name, size);
				// create the buffer
				buf = calloc(size, 1);
				if ( buf == NULL ) {
					printf("FATAL: unable to allocate ws buffer\n");
					return -1;
				}

				// fill the buffer
				printf("==> copy name\n");
				abuf = &buf[LWS_SEND_BUFFER_PRE_PADDING];
				memcpy(abuf, func->name, strlen(func->name) + 1);
				abuf += strlen(func->name) + 1;
				printf("==> copy args (argc=%u, sizeof=%d)\n", argc, sizeof(argc));
				memcpy(abuf, &argc, sizeof(argc));
				abuf += sizeof(argc);
				arg = func->args;
				while ( arg ) {
					memcpy(abuf, &arg->size, sizeof(int));
					abuf += sizeof(int);
					memcpy(abuf, arg->data, arg->size);
					abuf += arg->size;
					arg = arg->next;
				}

				// serialize and send to socket
				printf("==> sending %s with %d data\n", func->name, size);
				hexdump(buf, size);
				n = libwebsocket_write(wsi, (unsigned char *)buf, size, LWS_WRITE_BINARY);
				printf("    <-- n was %d\n", n);

				f_free(func);
			}


			break;

		default:
			break;
	}

	return 0;
}

static struct libwebsocket_protocols protocols[] = {

	[PROTOCOL_GLPROXY] = {
		.name = "glproxy",
		.callback = callback_glproxy,
		//.per_session_data_size = sizeof (struct per_session_data__ping),
	},
	[DEMO_PROTOCOL_COUNT] = {  /* end of list */
		.callback = NULL
	}
};

void *ws_thread(void *arg) {
	struct libwebsocket_context *context;
	int n = 0;

	context = libwebsocket_create_context(server_port, NULL, protocols,
		libwebsocket_internal_extensions, NULL, NULL, -1, -1, 0);

	if (context == NULL) {
		fprintf(stderr, "libwebsocket init failed\n");
		exit(1);
	}

	while (n >= 0) {
		n = libwebsocket_service(context, 50);

		pthread_mutex_lock(&qfuncmtx);
		if ( qfunc != NULL )
			libwebsocket_callback_on_writable_all_protocol(protocols);
		//printf("qlen = %d\n", qlen);
		pthread_mutex_unlock(&qfuncmtx);
	}

	libwebsocket_context_destroy(context);

	return 0;
}

void ws_start() {
    pthread_t thread;// On crée un thread
    pthread_create(&thread, NULL, ws_thread, NULL);// Permet d'exécuter le fonction maFonction en parallèle
}

/**
int main(int argc, char **argv) {
	ws_start();
	while (1)
		sleep(1);
	return 0;
}
**/

__attribute__((constructor))
static void __ws_init(void) {
	printf("--> glproxy loading...\n");
	ws_start();
	printf("--> glproxy loaded.\n");
}