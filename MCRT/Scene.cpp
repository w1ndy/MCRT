#include "Scene.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

using namespace std;

template<typename Out>
inline void split(const string &s, char delim, Out result)
{
	stringstream ss;
	ss.str(s);
	string item;
	while (getline(ss, item, delim)) {
		*(result++) = item;
	}
}

inline vector<string> split(const string &s, char delim)
{
	vector<string> elems;
	split(s, delim, back_inserter(elems));
	return elems;
}

inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

inline vector<vector<string>> processFaceArgs(string &faceArgs)
{
	vector<string> vertices = split(ltrim(faceArgs), ' ');
	vector<vector<string>> result;
	
	for (auto &v : vertices) {
		result.push_back(split(v, '/'));
	}
	return result;
}

inline int convertStringToIndex(const string &s)
{
	if (s.length() == 0) return -1;
	return atoi(s.c_str()) - 1;
}

Scene::Scene(const char * objFileName)
{
	if (!readFromObjFile(objFileName)) {
		clear();
	}
}

void Scene::clear()
{
	_groups.clear();
	_vertices.clear();
	_textureCoords.clear();
	_normals.clear();
	_faces.clear();
	_matLib.clear();
}

bool Scene::readFromObjFile(const char * objFileName)
{
	fstream fin(objFileName);
	if (!fin.is_open()) {
		cout << "Error: unable to open obj file: "
			<< objFileName << endl;
		return false;
	}

	SceneGroup *currentGroup = nullptr;
	Vertex3f v;
	TextureCoord c;
	bool smoothMode = false;
	Material *mat = nullptr;
	string op, arg;
	vector<vector<string>> faceArgs;
	TriangleFace f;

	while (fin >> op) {
		if (op.length() == 0) break;
		if (op[0] == '#') {
			getline(fin, op);
		}
		else if (op == "mtllib") {
			fin >> arg;
			_matLib.readFromMtlFile(arg.c_str());
		}
		else if (op == "g") {
			_groups.push_back(SceneGroup());
			currentGroup = &_groups[_groups.size() - 1];
			fin >> currentGroup->name;
		}
		else if (op == "v") {
			fin >> v.x >> v.y >> v.z;
			_vertices.push_back(v);
			if (currentGroup) {
				currentGroup->vertices.push_back(
					&_vertices[_vertices.size() - 1]);
			}
		}
		else if (op == "vt") {
			fin >> c.u >> c.v;
			_textureCoords.push_back(c);
			if (currentGroup) {
				currentGroup->textureCoords.push_back(
					&_textureCoords[_textureCoords.size() - 1]);
			}
		}
		else if (op == "vn") {
			fin >> v.x >> v.y >> v.z;
			_normals.push_back(v);
			if (currentGroup) {
				currentGroup->normals.push_back(
					&_normals[_normals.size() - 1]);
			}
		}
		else if (op == "s") {
			fin >> arg;
			if (arg == "1") {
				smoothMode = true;
			} else {
				smoothMode = false;
			}
		}
		else if (op == "usemtl") {
			fin >> arg;
			mat = _matLib.getMaterialByName(arg.c_str());
			if (mat == nullptr) {
				cout << "Warn: material not found: " << arg << endl;
			}
		}
		else if (op == "f") {
			getline(fin, arg);
			faceArgs = processFaceArgs(arg);
			size_t faceNum = faceArgs.size();
			size_t compNum = faceArgs[0].size();
			assert(faceNum >= 3 && compNum >= 1 && "invalid face description");
			f.v1 = convertStringToIndex(faceArgs[0][0]);
			f.vt1 = compNum < 2 ? -1 : convertStringToIndex(faceArgs[0][1]);
			f.vn1 = compNum < 3 ? -1 : convertStringToIndex(faceArgs[0][2]);
			f.smooth = smoothMode;
			f.mat = mat;
			for (int i = 1; i + 1 < faceArgs.size(); i++) {
				assert(faceArgs[i].size() == compNum &&
					faceArgs[i + 1].size() == compNum &&
					"invalid face description");
				f.v2 = convertStringToIndex(faceArgs[i][0]);
				f.v3 = convertStringToIndex(faceArgs[i + 1][0]);
				f.vt2 = compNum < 2 ? -1 : convertStringToIndex(faceArgs[i][1]);
				f.vt3 = compNum < 2 ? -1 : convertStringToIndex(faceArgs[i + 1][1]);
				f.vn2 = compNum < 3 ? -1 : convertStringToIndex(faceArgs[i][2]);
				f.vn3 = compNum < 3 ? -1 : convertStringToIndex(faceArgs[i + 1][2]);
				_faces.push_back(f);
				if (currentGroup) {
					currentGroup->faces.push_back(&_faces[_faces.size() - 1]);
				}
			}
		}
		else {
			cout << "Warn: unrecognized obj file operator: "
				<< op << endl;
			getline(fin, op);
		}
	}

	return false;
}

