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

	MaterialData() : diffuse{ 0. }, ambient{ 0. }, specular{ 0. }, specularExponent(0), transparency(1.0) {};
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
