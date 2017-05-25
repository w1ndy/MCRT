#pragma once

#include "MaterialLibrary.h"

#include <vector>
#include <string>

#include "Vector.h"

struct TriangleFace
{
	int v1, v2, v3;
	int vt1, vt2, vt3;
	int vn1, vn2, vn3;
	bool smooth;
	Material *mat;
};

struct SceneGroup
{
	std::string name;
	std::vector<Vertex3f *> vertices;
	std::vector<TextureCoord *> textureCoords;
	std::vector<Vertex3f *> normals;
	std::vector<TriangleFace *> faces;
};

class Scene
{
private:
	MaterialLibrary _matLib;
	std::vector<SceneGroup> _groups;
	std::vector<Vertex3f> _vertices;
	std::vector<TextureCoord> _textureCoords;
	std::vector<Vertex3f> _normals;
	std::vector<TriangleFace> _faces;

public:
	Scene() {};
	Scene(const char *objFileName);

	void clear();
	bool readFromObjFile(const char *objFileName);
};
