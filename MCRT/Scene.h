#pragma once

#include "MaterialLibrary.h"
#include "Types.h"

#include <vector>
#include <string>


struct TriangleFace
{
	int v1, v2, v3;
	int vt1, vt2, vt3;
	int vn1, vn2, vn3;
};

struct SceneGroup
{
	std::string name;
	std::vector<Eigen::Vector3f *> vertices;
	std::vector<Eigen::Vector2f *> textureCoords;
	std::vector<Eigen::Vector3f *> normals;
	std::vector<TriangleFace *> faces;
	bool smooth;
	Material *mat;
};

class Scene
{
private:
	MaterialLibrary _matLib;
	std::vector<SceneGroup> _groups;
	std::vector<Eigen::Vector3f> _vertices;
	std::vector<Eigen::Vector2f> _textureCoords;
	std::vector<Eigen::Vector3f> _normals;
	std::vector<TriangleFace> _faces;

public:
	Scene() {};
	Scene(const char *objFileName);

	void clear();
	bool readFromObjFile(const char *objFileName);
	void computeNormals();

	/*
	* Free after use
	* groups: vertices index | normals index | vertex count | material index
	* vertices: vertices | normals
	* materials: MaterialData (11 * float)
	*/
	void getGroupBuffers(int **grpBuf, float **vtxBuf, float **matBuf);
};
