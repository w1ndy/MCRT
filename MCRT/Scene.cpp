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
using namespace Eigen;

template<typename Out>
inline void split(const string &s, char delim, Out result, bool skipEmpty)
{
	stringstream ss;
	ss.str(s);
	string item;
	while (getline(ss, item, delim)) {
		if (!skipEmpty || item.length() != 0) {
			*(result++) = item;
		}
	}
}

inline vector<string> split(const string &s, char delim, bool skipEmpty = true)
{
	vector<string> elems;
	split(s, delim, back_inserter(elems), skipEmpty);
	return elems;
}

inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

inline vector<vector<string>> processFaceArgs(string &faceArgs)
{
	vector<string> vertices = split(faceArgs, ' ');
	vector<vector<string>> result;
	
	for (auto &v : vertices) {
		result.push_back(split(v, '/', false));
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
	for (auto g : _groups) delete g;
	_groups.clear();
	for (auto v : _vertices) delete v;
	_vertices.clear();
	for (auto c : _textureCoords) delete c;
	_textureCoords.clear();
	for (auto n : _normals) delete n;
	_normals.clear();
	for (auto f : _faces) delete f;
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

	SceneGroup *currentGroup = new SceneGroup;
	currentGroup->name = "default";
	_groups.push_back(currentGroup);

	float x, y, z;
	Vector2f c;
	bool smoothMode = false;
	Material *mat = nullptr;
	string op, arg;
	vector<vector<string>> faceArgs;

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
			fin >> arg;
			if (!currentGroup || currentGroup->name != arg) {
				currentGroup = new SceneGroup;
				currentGroup->name = arg;
				_groups.push_back(currentGroup);
			}
		}
		else if (op == "v") {
			fin >> x >> y >> z;
			Vector3f *pvec3 = new Vector3f(x, y, z);
			_vertices.push_back(pvec3);
			if (currentGroup) {
				currentGroup->vertices.push_back(pvec3);
			}
		}
		else if (op == "vt") {
			fin >> c[0] >> c[1];
			Vector2f *pvec2 = new Vector2f(x, y);
			_textureCoords.push_back(pvec2);
			if (currentGroup) {
				currentGroup->textureCoords.push_back(pvec2);
			}
		}
		else if (op == "vn") {
			fin >> x >> y >> z;
			Vector3f *pvec3 = new Vector3f(x, y, z);
			_normals.push_back(pvec3);
			if (currentGroup) {
				currentGroup->normals.push_back(pvec3);
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
			if (currentGroup) {
				currentGroup->smooth = smoothMode;
				currentGroup->mat = mat;
			}
			for (size_t i = 1; i + 1 < faceArgs.size(); i++) {
				assert(faceArgs[i].size() == compNum &&
					faceArgs[i + 1].size() == compNum &&
					"invalid face description");
				TriangleFace *f = new TriangleFace;
				f->v1 = convertStringToIndex(faceArgs[0][0]);
				f->v2 = convertStringToIndex(faceArgs[i][0]);
				f->v3 = convertStringToIndex(faceArgs[i + 1][0]);
				f->vt1 = compNum < 2 ? -1 : convertStringToIndex(faceArgs[0][1]);
				f->vt2 = compNum < 2 ? -1 : convertStringToIndex(faceArgs[i][1]);
				f->vt3 = compNum < 2 ? -1 : convertStringToIndex(faceArgs[i + 1][1]);
				f->vn1 = compNum < 3 ? -1 : convertStringToIndex(faceArgs[0][2]);
				f->vn2 = compNum < 3 ? -1 : convertStringToIndex(faceArgs[i][2]);
				f->vn3 = compNum < 3 ? -1 : convertStringToIndex(faceArgs[i + 1][2]);
				_faces.push_back(f);
				if (currentGroup) {
					currentGroup->faces.push_back(f);
				}
			}
		}
		else {
			cout << "Warn: unrecognized obj file operator: "
				<< op << endl;
			getline(fin, op);
		}
	}

	return true;
}

void Scene::computeNormals()
{
	if (_normals.size()) return;

	for (auto &v : _vertices) {
		_normals.push_back(new Vector3f(0, 0, 0));
	}

	int *count = new int[_normals.size()];
	memset(count, 0, sizeof(int) * _normals.size());

	for (auto &f : _faces) {
		Vector3f u = *_vertices[f->v2] - *_vertices[f->v1];
		Vector3f v = *_vertices[f->v3] - *_vertices[f->v1];
		Vector3f n = u.cross(v).normalized();
		*_normals[f->v1] += n;
		*_normals[f->v2] += n;
		*_normals[f->v3] += n;
		count[f->v1]++;
		count[f->v2]++;
		count[f->v3]++;
		f->vn1 = f->v1;
		f->vn2 = f->v2;
		f->vn3 = f->v3;
	}
	for (size_t i = 0; i < _normals.size(); i++) {
		if (count[i]) *_normals[i] /= (float)count[i];
	}
	delete count;
}

void Scene::getGroupBuffers(
	int **grpBuf, size_t *grpBufLen,
	float **vtxBuf, size_t *vtxBufLen,
	float **bboxBuf, size_t *bboxBufLen,
	float **matBuf, size_t *matBufLen)
{
	const int VerticesPerFace = 3;
	const int FloatsPerVertex = 3;
	const int FloatsPerMaterial = 11;
	const int GroupBufferStride = 4;

	MaterialData defaultMat;

	*vtxBufLen = 0, *bboxBufLen = 0, *matBufLen = 0;

	*grpBufLen = GroupBufferStride * _groups.size();
	*grpBuf = new int[*grpBufLen];
	for (int i = 0; i < _groups.size(); i++) {
		int offset = FloatsPerVertex * VerticesPerFace * _groups[i]->faces.size();
		(*grpBuf)[i * GroupBufferStride] = *vtxBufLen;
		*vtxBufLen += offset;
		(*grpBuf)[i * GroupBufferStride + 1] = *vtxBufLen;
		*vtxBufLen += offset;
		(*grpBuf)[i * GroupBufferStride + 2] = offset / FloatsPerVertex;
		(*grpBuf)[i * GroupBufferStride + 3] = *matBufLen;
		*matBufLen += FloatsPerMaterial;
		*bboxBufLen += 2 * FloatsPerVertex;
	}

	*vtxBuf = new float[*vtxBufLen];
	*bboxBuf = new float[*bboxBufLen];
	*matBuf = new float[*matBufLen];

	for (int i = 0; i < *bboxBufLen; i++) {
		if (i % 6 < 3) (*bboxBuf)[i] = 999999;
		else  (*bboxBuf)[i] = -999999;
	}

	int vbptr = 0, bbptr = 0, mbptr = 0;
	for (auto &g : _groups) {
		for (auto &f : g->faces) {
			(*bboxBuf)[bbptr] = min({ (*bboxBuf)[bbptr],
				_vertices[f->v1]->x(), _vertices[f->v2]->x(), _vertices[f->v3]->x() });
			(*bboxBuf)[bbptr + 1] = min({ (*bboxBuf)[bbptr + 1],
				_vertices[f->v1]->y(), _vertices[f->v2]->y(), _vertices[f->v3]->y() });
			(*bboxBuf)[bbptr + 2] = min({ (*bboxBuf)[bbptr + 2],
				_vertices[f->v1]->z(), _vertices[f->v2]->z(), _vertices[f->v3]->z() });
			memcpy(*vtxBuf + vbptr, _vertices[f->v1]->data(), 3 * sizeof(float));
			memcpy(*vtxBuf + vbptr + 3, _vertices[f->v2]->data(), 3 * sizeof(float));
			memcpy(*vtxBuf + vbptr + 6, _vertices[f->v3]->data(), 3 * sizeof(float));
			vbptr += FloatsPerVertex * VerticesPerFace;
		}
		bbptr += 3;
		for (auto &f : g->faces) {
			(*bboxBuf)[bbptr] = max({ (*bboxBuf)[bbptr],
				_vertices[f->v1]->x(), _vertices[f->v2]->x(), _vertices[f->v3]->x() });
			(*bboxBuf)[bbptr + 1] = max({ (*bboxBuf)[bbptr + 1],
				_vertices[f->v1]->y(), _vertices[f->v2]->y(), _vertices[f->v3]->y() });
			(*bboxBuf)[bbptr + 2] = max({ (*bboxBuf)[bbptr + 2],
				_vertices[f->v1]->z(), _vertices[f->v2]->z(), _vertices[f->v3]->z() });
			memcpy(*vtxBuf + vbptr, _normals[f->vn1]->data(), 3 * sizeof(float));
			memcpy(*vtxBuf + vbptr + 3, _normals[f->vn2]->data(), 3 * sizeof(float));
			memcpy(*vtxBuf + vbptr + 6, _normals[f->vn3]->data(), 3 * sizeof(float));
			vbptr += FloatsPerVertex * VerticesPerFace;
		}
		bbptr += 3;
		if (g->mat) {
			memcpy(*matBuf + mbptr, &(g->mat->data), sizeof(MaterialData));
		} else {
			memcpy(*matBuf + mbptr, &defaultMat, sizeof(MaterialData));
		}
		mbptr += FloatsPerMaterial;
	}
}


