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
				fin >> currentMat->data.diffuse[0] >> currentMat->data.diffuse[1]
					>> currentMat->data.diffuse[2];
			}
			else if (op == "Ka") {
				fin >> currentMat->data.ambient[0] >> currentMat->data.ambient[1]
					>> currentMat->data.ambient[2];
			}
			else if (op == "Ks") {
				fin >> currentMat->data.specular[0] >> currentMat->data.specular[1]
					>> currentMat->data.specular[2];
			}
			else if (op == "Tr") {
				fin >> currentMat->data.transparency;
			}
			else if (op == "d") {
				fin >> currentMat->data.transparency;
				currentMat->data.transparency = 1 - currentMat->data.transparency;
			}
			else if (op == "Ns") {
				fin >> currentMat->data.specularExponent;
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
