#include "Tracer.h"

#include <iostream>
#include <GL/freeglut.h>

using namespace std;

Tracer *Tracer::_instance;

Tracer::Tracer(int &argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutInitContextVersion(4, 2);
	glutInitContextFlags(GLUT_CORE_PROFILE);
	glutCreateWindow("MCRT");
	glutDisplayFunc(_displayFn);
	glutReshapeFunc(_resizeFn);
	glutIdleFunc(_idleFn);

	Tracer::_instance = this;
}

void Tracer::run()
{
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}

void Tracer::onUpdating()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
	cout << "rendered a frame" << endl;
}

void Tracer::onResized(int width, int height)
{
	if (width == _width && height == _height) return;
	_width = width;
	_height = height;

	glViewport(0, 0, width, height);
}

void Tracer::onIdle()
{
	glutPostRedisplay();
}

void Tracer::_displayFn()
{
	Tracer::_instance->onUpdating();
}

void Tracer::_resizeFn(int width, int height)
{
	Tracer::_instance->onResized(width, height);
}

void Tracer::_idleFn()
{
	Tracer::_instance->onIdle();
}
