#pragma once

#include "Types.h"

class Tracer
{
public:
	Tracer(int &argc, char *argv[]);

	void run();

private:
	void _onUpdating();
	void _onResized(int width, int height);
	void _onIdle();

private:
	void _setFrustum(float fovy, float aspect, float zNear, float zFar);
	void _setCamera(
		const Eigen::Vector3f &eye,
		const Eigen::Vector3f &at,
		const Eigen::Vector3f &up);
	void _computeInvMatrix();
	void _buildCanvas();
	void _buildVertexArray();
	void _loadShaders();

private:
	int _width;
	int _height;
	unsigned int _canvas, _canvasVertexArray;
	unsigned int _computeProgram, _renderProgram;
	Eigen::Vector3f _cop;
	Eigen::Matrix4f _viewMat, _projMat, _invMat;

private:
	static Tracer *_instance;
	static void _displayFn();
	static void _resizeFn(int width, int height);
	static void _idleFn();
};
