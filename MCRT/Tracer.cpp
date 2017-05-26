#include "Tracer.h"

#include <iostream>
#include <cmath>
#include <chrono>
#include <random>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;
using namespace Eigen;

Tracer *Tracer::_instance;

Tracer::Tracer(int &argc, char *argv[]) : _canvas(0), _frames(0)
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

void Tracer::run(Scene &s)
{
	cout << "MCRT has started." << endl;
	_buildSSBOs(s);
	_buildVertexArray();
	_loadShaders();
	_initShaders();

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}

float getRandomOffset() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<> dist(-0.5, 0.5);
	return dist(gen);
}

int nextPower2(int x) {
	if (x == 0) return 1;
	x -= 1;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x += 1;
	return x;
}

void Tracer::_onUpdating()
{
	_frames++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto start = chrono::steady_clock::now();

	glUseProgram(_computeProgram);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _ssbo.materials);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _ssbo.bboxes);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _ssbo.groups);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _ssbo.vertices);

	glUniform3f(_variables.eye, _cop[0], _cop[1], _cop[2]);
	glUniform3f(_variables.ray00, _ray00[0], _ray00[1], _ray00[2]);
	glUniform3f(_variables.ray01, _ray01[0], _ray01[1], _ray01[2]);
	glUniform3f(_variables.ray10, _ray10[0], _ray10[1], _ray10[2]);
	glUniform3f(_variables.ray11, _ray11[0], _ray11[1], _ray11[2]);

	glUniform2f(_variables.sampleOffset, getRandomOffset(), getRandomOffset());
	glUniform1i(_variables.frame, _frames);
	glBindImageTexture(0, _canvas, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(
		nextPower2(_width) / _groupSizeX,
		nextPower2(_height) / _groupSizeY,
		1);

	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(_renderProgram);
	glBindVertexArray(_canvasVertexArray);
	glBindTexture(GL_TEXTURE_2D, _canvas);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glutSwapBuffers();

	auto end = chrono::steady_clock::now();
	auto ms = chrono::duration<double, milli>(end - start).count();
	cout << "Frame #" << _frames << " costs "
		<< ms << " ms, est " << (1000.0 / ms) << " fps" << endl;
}

void Tracer::_onResized(int width, int height)
{
	if (width == _width && height == _height) return;
	cout << "resizing to " << width << 'x' << height << endl;

	_width = width;
	_height = height;

	float aspect = height != 0 ? float(width) / float(height) : width;

	glViewport(0, 0, width, height);
	_setFrustum(60, aspect, 1., 20.);
	_setCamera(
		Vector3f(0, 0, 10),
		Vector3f(0, 0, -1),
		Vector3f(0, 1, 0));
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
	_ray00 = _getRayVector(-1, -1);
	_ray01 = _getRayVector(-1, 1);
	_ray10 = _getRayVector(1, -1);
	_ray11 = _getRayVector(1, 1);
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

GLuint createSSBO(void *buffer, size_t size)
{
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, buffer, GL_STATIC_READ);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return ssbo;
}

void Tracer::_buildSSBOs(Scene &s)
{
	int *grpBuf;
	float *vtxBuf, *bboxBuf, *matBuf;
	size_t grpBufLen, vtxBufLen, bboxBufLen, matBufLen;

	s.getGroupBuffers(
		&grpBuf, &grpBufLen,
		&vtxBuf, &vtxBufLen,
		&bboxBuf, &bboxBufLen,
		&matBuf, &matBufLen);

	_ssbo.groups = createSSBO((void *)grpBuf, grpBufLen * sizeof(int));
	_ssbo.vertices = createSSBO((void *)vtxBuf, vtxBufLen * sizeof(float));
	_ssbo.bboxes = createSSBO((void *)bboxBuf, bboxBufLen * sizeof(float));
	_ssbo.materials = createSSBO((void *)matBuf, matBufLen * sizeof(float));

	delete grpBuf;
	delete vtxBuf;
	delete bboxBuf;
	delete matBuf;
}

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

void Tracer::_initShaders()
{
	glUseProgram(_computeProgram);
	_variables.eye = glGetUniformLocation(_computeProgram, "eye");
	_variables.ray00 = glGetUniformLocation(_computeProgram, "ray00");
	_variables.ray01 = glGetUniformLocation(_computeProgram, "ray01");
	_variables.ray10 = glGetUniformLocation(_computeProgram, "ray10");
	_variables.ray11 = glGetUniformLocation(_computeProgram, "ray11");
	_variables.sampleOffset = glGetUniformLocation(_computeProgram, "sample_offset");
	_variables.frame = glGetUniformLocation(_computeProgram, "frame");

	glUseProgram(_renderProgram);
	_variables.tex = glGetUniformLocation(_renderProgram, "tex");
	glUniform1i(_variables.tex, 0);

	glUseProgram(NULL);
}

Vector3f Tracer::_getRayVector(float x, float y)
{
	Vector4f r = _invMat * Vector4f(x, y, 0, 1);
	r[0] /= r[3];
	r[1] /= r[3];
	r[2] /= r[3];
	return Vector3f(r[0], r[1], r[2]);
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
