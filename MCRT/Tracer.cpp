#include "Tracer.h"

#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;
using namespace Eigen;

Tracer *Tracer::_instance;

GLuint compileShader(const char *fname, GLenum type)
{
	GLuint shader = glCreateShader(type);
	FILE *fp = fopen(fname, "rb");
	if (fp == NULL) {
		cout << "Error: cannot open shader: " << fname << endl;
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	GLint length = ftell(fp);
	GLchar *buf = new GLchar[length + 1];
	fseek(fp, 0, SEEK_SET);
	fread(buf, sizeof(GLchar), length, fp);
	buf[length] = 0;
	fclose(fp);

	glShaderSource(shader, 1, (const GLchar **)&buf, &length);
	glCompileShader(shader);
	delete buf;

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		char *logbuf = new char[2048];
		GLsizei loglen;
		glGetShaderInfoLog(shader, 2048, &loglen, logbuf);
		logbuf[loglen] = 0;
		cout << "Error: unable to compile shader:" << endl
			<< logbuf << endl;
		delete logbuf;
		return 0;
	}

	return shader;
}

Tracer::Tracer(int &argc, char *argv[]) : _canvas(0)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutInitContextVersion(4, 2);
	glutInitContextFlags(GLUT_CORE_PROFILE);
	glutCreateWindow("MCRT");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		cout << "Error: unable to initialize glew: "
			<< glewGetErrorString(err) << endl;
	}

	glutDisplayFunc(_displayFn);
	glutReshapeFunc(_resizeFn);
	glutIdleFunc(_idleFn);

	Tracer::_instance = this;
}

void Tracer::run()
{
	cout << "MCRT has started." << endl;
	_buildVertexArray();
	_loadShaders();

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}

void Tracer::_onUpdating()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
	cout << "rendered a frame" << endl;
}

void Tracer::_onResized(int width, int height)
{
	if (width == _width && height == _height) return;
	cout << "resizing to " << width << 'x' << height << endl;

	_width = width;
	_height = height;

	float aspect = height != 0 ? float(width) / float(height) : width;

	glViewport(0, 0, width, height);
	_setFrustum(60, aspect, 1., 2.);
	_setCamera(
		Vector3f(-1.8f, -1.8f, 0.6f),
		Vector3f(0, 0, 0.6f),
		Vector3f(0, 0, 1.0f));
	_computeInvMatrix();
	_buildCanvas();
}

void Tracer::_onIdle()
{
	//glutPostRedisplay();
}

void Tracer::_setFrustum(float fovy, float aspect, float zNear, float zFar)
{
	float halfH = tanf(fovy / 180.0f * 3.14159f * 0.5f) * zNear;
	float halfW = aspect * halfH;
	float zL = zNear - zFar;
	_projMat << zNear / halfW, 0, 0, 0,
				0, zNear / halfH, 0, 0,
				0, 0, (zFar + zNear) / zL, 2.0f * zFar * zNear / zL,
				0, 0, -1, 0;
}

void Tracer::_setCamera(const Vector3f &eye, const Vector3f &at, const Vector3f &up)
{
	_cop = eye;
	Vector3f dn = (at - eye).normalized();
	Vector3f un = up.normalized();
	Vector3f rn = dn.cross(un).normalized();
	_viewMat << rn(0), rn(1), rn(2), -rn.dot(eye),
				un(0), un(1), un(2), -un.dot(eye),
				-dn(0), -dn(1), -dn(2), dn.dot(eye),
				0, 0, 0, 1;
}

void Tracer::_computeInvMatrix()
{
	_invMat = (_projMat * _viewMat).reverse();
}

void Tracer::_buildCanvas()
{
	if (_canvas) glDeleteTextures(1, &_canvas);
	glGenTextures(1, &_canvas);
	glBindTexture(GL_TEXTURE_2D, _canvas);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _width, _height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Tracer::_buildVertexArray()
{
	GLuint bufferObject;
	glGenVertexArrays(1, &_canvasVertexArray);
	glGenBuffers(1, &bufferObject);
	glBindVertexArray(_canvasVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
	
	const float vertices[] = { -1, -1, 1, -1, 1, 1, 1, 1, -1, 1, -1, -1 };
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), (const GLvoid *)vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(NULL);
}

void Tracer::_loadShaders()
{
	GLuint fs = compileShader("quad.frag", GL_FRAGMENT_SHADER);
	GLuint vs = compileShader("quad.vert", GL_VERTEX_SHADER);
	GLuint cs = compileShader("trace.comp", GL_COMPUTE_SHADER);

	_computeProgram = glCreateProgram();
	glAttachShader(_computeProgram, cs);
	glLinkProgram(_computeProgram);

	_renderProgram = glCreateProgram();
	glAttachShader(_renderProgram, vs);
	glAttachShader(_renderProgram, fs);
	glLinkProgram(_renderProgram);
}

void Tracer::_displayFn()
{
	Tracer::_instance->_onUpdating();
}

void Tracer::_resizeFn(int width, int height)
{
	Tracer::_instance->_onResized(width, height);
}

void Tracer::_idleFn()
{
	Tracer::_instance->_onIdle();
}
