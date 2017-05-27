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
	bool smooth;
	Material *mat;

	TriangleFace() : mat(0), smooth(false) {};
};

struct SceneGroup
{
	std::string name;
	std::vector<Eigen::Vector3f *> vertices;
	std::vector<Eigen::Vector2f *> textureCoords;
	std::vector<Eigen::Vector3f *> normals;
	std::vector<TriangleFace *> faces;

	SceneGroup() {};
};

struct face_t {
	// geometry
	float v1[4];
	float v2[4];
	float v3[4];
	float vn1[4];
	float vn2[4];
	float vn3[4];

	// material
	float Kd[4];
	float Ka[4];
	float Ks[4];
	float Ns;
	float Tr;
	float _padding[2];

	void setVertices(
		const Eigen::Vector3f &v1,
		const Eigen::Vector3f &v2,
		const Eigen::Vector3f &v3)
	{
		memcpy(this->v1, v1.data(), 3 * sizeof(float));
		memcpy(this->v2, v2.data(), 3 * sizeof(float));
		memcpy(this->v3, v3.data(), 3 * sizeof(float));
	}

	void setNormals(
		const Eigen::Vector3f &vn1,
		const Eigen::Vector3f &vn2,
		const Eigen::Vector3f &vn3)
	{
		memcpy(this->vn1, vn1.data(), 3 * sizeof(float));
		memcpy(this->vn2, vn2.data(), 3 * sizeof(float));
		memcpy(this->vn3, vn3.data(), 3 * sizeof(float));
	}

	void setMaterial(const MaterialData &d)
	{
		Kd[0] = d.diffuse[0], Kd[1] = d.diffuse[1], Kd[2] = d.diffuse[2];
		Ka[0] = d.ambient[0], Ka[1] = d.ambient[1], Ka[2] = d.ambient[2];
		Ks[0] = d.specular[0], Ks[1] = d.specular[1], Ks[2] = d.specular[2];
		Ns = d.specularExponent;
		Tr = d.transparency;
	}
};

struct group_t {
	int fptr;
	int flen;
	int _padding[2];
	float vmin[4];
	float vmax[4];

	group_t() : vmin{ 99999 }, vmax{ -99999 } {};

	void updateBoundingBox(
		const Eigen::Vector3f &v1,
		const Eigen::Vector3f &v2,
		const Eigen::Vector3f &v3)
	{
		vmin[0] = std::min({ vmin[0], v1.x(), v2.x(), v3.x() });
		vmin[1] = std::min({ vmin[1], v1.y(), v2.y(), v3.y() });
		vmin[2] = std::min({ vmin[2], v1.z(), v2.z(), v3.z() });
		vmax[0] = std::max({ vmax[0], v1.x(), v2.x(), v3.x() });
		vmax[1] = std::max({ vmax[1], v1.y(), v2.y(), v3.y() });
		vmax[2] = std::max({ vmax[2], v1.z(), v2.z(), v3.z() });
	}
};

class Scene
{
private:
	MaterialLibrary _matLib;
	std::vector<SceneGroup *> _groups;
	std::vector<Eigen::Vector3f *> _vertices;
	std::vector<Eigen::Vector2f *> _textureCoords;
	std::vector<Eigen::Vector3f *> _normals;
	std::vector<TriangleFace *> _faces;

public:
	Scene() {};
	Scene(const char *objFileName);

	void clear();
	bool readFromObjFile(const char *objFileName);
	void computeNormals();

	void getGroupBuffers(
		group_t **grpBuf, size_t *grpBufLen,
		face_t **faceBuf, size_t *faceBufLen);
};
