#pragma once

#include "Types.h"
#include "Scene.h"

class Tracer
{
public:
	Tracer(int &argc, char *argv[]);

	void run(Scene &s);

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
	void _buildSSBOs(Scene &s);
	void _loadShaders();
	void _initShaders();
	Eigen::Vector3f _getRayVector(float x, float y);

private:
	int _frames;
	int _width;
	int _height;
	unsigned int _canvas, _canvasVertexArray;
	unsigned int _computeProgram, _renderProgram;
	Eigen::Vector3f _cop, _ray00, _ray01, _ray10, _ray11;
	Eigen::Matrix4f _viewMat, _projMat, _invMat;

	struct SSBOCollection {
		unsigned int groups;
		unsigned int vertices;
		unsigned int bboxes;
		unsigned int materials;
	} _ssbo;

	struct ShaderVariableCollection {
		unsigned int eye;
		unsigned int ray00;
		unsigned int ray01;
		unsigned int ray10;
		unsigned int ray11;
		unsigned int sampleOffset;
		unsigned int frame;
		unsigned int tex;
	} _variables;

private:
	const unsigned int _groupSizeX = 16;
	const unsigned int _groupSizeY = 8;

private:
	static Tracer *_instance;
	static void _displayFn();
	static void _resizeFn(int width, int height);
	static void _idleFn();
};
