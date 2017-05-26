#pragma once

#include <string>
#include <vector>
#include <map>

struct MaterialData
{
	float diffuse[3];
	float ambient[3];
	float specular[3];
	float specularExponent;
	float transparency;
};

struct Material
{
	std::string name;
	int illum;
	MaterialData data;
};

class MaterialLibrary
{
private:
	std::vector<Material *> _mats;
	std::map<std::string, Material *> _index;

public:
	MaterialLibrary() {};
	MaterialLibrary(const char *mtlFileName);

	void clear();
	bool readFromMtlFile(const char *mtlFileName);
	Material *getMaterialByName(const char *mtlName);
};
