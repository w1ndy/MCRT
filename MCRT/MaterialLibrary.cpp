#include "MaterialLibrary.h"

#include <iostream>
#include <fstream>

using namespace std;

MaterialLibrary::MaterialLibrary(const char *mtlFileName)
{
	if (!readFromMtlFile(mtlFileName)) {
		clear();
	}
}

void MaterialLibrary::clear()
{
	for (auto ptr : _mats) {
		delete ptr;
	}
	_mats.clear();
	_index.clear();
}

bool MaterialLibrary::readFromMtlFile(const char *mtlFileName)
{
	Material *currentMat = nullptr;
	string op;

	ifstream fin(mtlFileName);

	if (!fin.is_open()) {
		cout << "Error: Failed to open mtl: " << mtlFileName << endl;
		return false;
	}

	while (fin >> op) {
		if (op.length() == 0) break;
		
		if (op == "newmtl") {
			currentMat = new Material;
			fin >> currentMat->name;
			if (_index.find(currentMat->name) != _index.end()) {
				cout << "Error: material name is not unique: "
					<< currentMat->name << endl;
				return false;
			}
			_mats.push_back(currentMat);
			_index[currentMat->name] = currentMat;
		}
		else if (!currentMat) {
			cout << "Error: material is not defined yet" << endl;
			return false;
		}
		else {
			if (op == "illum") {
				fin >> currentMat->illum;
			}
			else if (op == "Kd") {
				fin >> currentMat->diffuse[0] >> currentMat->diffuse[1]
					>> currentMat->diffuse[2];
			}
			else if (op == "Ka") {
				fin >> currentMat->ambient[0] >> currentMat->ambient[1]
					>> currentMat->ambient[2];
			}
			else if (op == "Ks") {
				fin >> currentMat->specular[0] >> currentMat->specular[1]
					>> currentMat->specular[2];
			}
			else if (op == "Tr") {
				fin >> currentMat->transparency;
			}
			else if (op == "d") {
				fin >> currentMat->transparency;
				currentMat->transparency = 1 - currentMat->transparency;
			}
			else if (op == "Ns") {
				fin >> currentMat->specularExponent;
			}
			else {
				cout << "Warn: undefined material operator: "
					<< op << endl;
				getline(fin, op);
			}
		}
	}
	return true;
}

Material * MaterialLibrary::getMaterialByName(const char * mtlName)
{
	auto it = _index.find(mtlName);
	return it == _index.end() ? nullptr : it->second;
}
